#include "gtb.h"

/*
	To-Do:

	If the setup function fails it cannot communicate the
	failure back to the library because put() wont work

	Notes:

	controller must be in blocking mode
*/

using namespace GTB;

Controller::Controller(Inputer& inputer_init, Outputer& outputer_init) : outputer(outputer_init), inputer(inputer_init)
{
	std::string path("/tw_lock_");
	path += std::to_string(pid);

	if(!(lock = sem_open(path.c_str(), O_CREAT, S_IRWXU | S_IRWXG, 1)))
    	throw FatalError("GTB: failed to create named semaphore");

    path = "/tmp/tw_in_";
    path += std::to_string(pid);

    if(mkfifo(path.data(), S_IRWXU | S_IRWXG) == -1)
    	throw FatalError("GTB: failed to create input FIFO pipe");

    //mark input as unsetup
    in = -1;

    path = "/tmp/tw_out_";
    path += std::to_string(pid);

    if(mkfifo(path.data(), S_IRWXU | S_IRWXG) == -1)
    	throw FatalError("GTB: failed to create output FIFO pipe");

    if((out = open(path.data(), O_RDONLY | O_NONBLOCK)) == -1)
    	throw FatalError("GTB: failed to open output FIFO pipe");

    io_channel = Glib::IOChannel::create_from_fd(out);

	Glib::RefPtr<Glib::IOSource> source = io_channel->create_watch(Glib::IOCondition::IO_IN | Glib::IOCondition::IO_PRI | Glib::IOCondition::IO_ERR | Glib::IOCondition::IO_HUP | Glib::IOCondition::IO_NVAL);

	source->connect(sigc::mem_fun(*this, &Controller::controller));
	source->set_priority(G_PRIORITY_DEFAULT); //Note using GTK C macro
	source->attach(Glib::MainContext::get_default());

    return;
}

Controller::~Controller()
{
	std::cout<<"~Controller\n";

	std::string path("/tw_lock_");
	path += std::to_string(pid);

	if(lock && sem_unlink(path.data()) == -1)
		std::cout<<"ERROR:GTB: failed to unlink named semaphore\n";

	if(in != -1)
	{
		//put("\021RET\022EXIT\0221\023");

		path = "/tmp/tw_in_";
		path += std::to_string(pid);

		if(close(in) == -1)
			std::cout<<"ERROR:GTB: failed to close input FIFO pipe\n";
	}

	if(unlink(path.data()) == -1)
		std::cout<<"ERROR:GTB: failed to unlink input FIFO pipe\n";

	path = "/tmp/tw_out_";
	path += std::to_string(pid);

	io_channel->close(true);

	if(unlink(path.data()) == -1)
		std::cout<<"ERROR:GTB: failed to unlink output FIFO pipe\n";

	return;
}

bool Controller::controller(const Glib::IOCondition cond)
{
	Glib::ustring str, ret;
	Glib::IOStatus status;
	Sequence seq;

	std::cout<<"\nin controller\n";

	if(cond & (Glib::IOCondition::IO_ERR | Glib::IOCondition::IO_NVAL))
	{
		std::cout<<"err\n";
		return false;
	}

	try
	{
		status = get(str);
		seq.parse(str);
	}
	catch(Error ex)
	{
		put(Glib::ustring("\021RET\022INVALID\0220\022").append(ex.what()).append("\023"));
		return true;
	}
	catch(FatalError ex)
	{
		throw ex;
	}
	catch(...)
	{
		put("\021RET\022INVALID\0220\022internal error\023");
		return true;
	}

	if(seq.arg.size())
	{
		try
		{
			ret = exec(seq);
		}
		catch(Error ex)
		{
			put(Glib::ustring("\021RET\022").append(seq.arg[0]).append("\0220\022").append(ex.what()).append("\023"));
			return true;
		}
		catch(FatalError ex)
		{
			throw ex;
		}
		catch(...)
		{
			put(Glib::ustring("\021RET\022").append(seq.arg[0]).append("\0220\022internal error\023"));
			return true;
		}

		put(ret);
	}

	if(status == Glib::IOStatus::IO_STATUS_EOF && cond & Glib::IOCondition::IO_HUP)
	{
		std::cout<<"hup && eof\n";
		return false;
	}

	return true;
}

Glib::IOStatus Controller::get(Glib::ustring& str)
{
	gunichar c = 0;
	gchar utf8_c[UTF8_CHAR_MAX + 1];
	Glib::IOStatus status;
	bool started = false;

	while(!started || c != DC3)
	{
		status = io_channel->read(c);

		if(status == Glib::IOStatus::IO_STATUS_EOF)
			break;
		else if(status != Glib::IOStatus::IO_STATUS_NORMAL)
			throw FatalError("GTB: failed to read DCS");

		if(started)
		{
			utf8_c[g_unichar_to_utf8(c, utf8_c)] = 0;
			str.append(utf8_c);
		}
		else if(c == DC1)
		{
			started = true;
			str.push_back(c);
		}
	}

	return status;
}

int Controller::put(Glib::ustring seq)
{
	ssize_t ret = 0;
	size_t nbytes = 0;

	//make sure 'in' is actually open
	//otherwise the child process is probably not
	//aware of the TW

	for(; nbytes != seq.size(); nbytes += ret)
		if((ret = write(in, seq.data(), seq.size())) == -1)
			throw FatalError("GTB: failed to write DCS");

	return 1;
}

bool Controller::scroll()
{
	outputer.win.view.scroll_to(scroll_pos);

	std::cout<<"controller scroll\n";

	//std::this_thread::sleep_for(std::chrono::seconds(2));

	return false;
}

Glib::ustring Controller::exec(Sequence seq)
{
	std::cout<<"exec "<<seq.arg[0]<<"\n";

	if(seq.arg[0] == "SETUP") return dcs_setup(seq);
	else if(seq.arg[0] == "MOVE") return dcs_move(seq);
	else if(seq.arg[0] == "FIND") return dcs_find(seq);
	else if(seq.arg[0] == "SCROLL") return dcs_scroll(seq);
	else if(seq.arg[0] == "INSERT") return dcs_insert(seq);
	else if(seq.arg[0] == "EDIT") return dcs_edit(seq);
	else if(seq.arg[0] == "CLEAR") return dcs_clear(seq);
	else if(seq.arg[0] == "EMBED") return dcs_embed(seq);
	else throw Error("unknown DCS");
}

//This function completes the setup process for the input pipe.
//The SETUP DCS is sent by every process connected to the GTB when
//it starts. This is done because when a POSIX FIFO (named pipe)
//is opened it will block unless another process has opened that pipe
//for reading.
Glib::ustring Controller::dcs_setup(Sequence seq)
{
	std::string path("/tmp/tw_in_");
	path += std::to_string(pid);

	if(seq.arg.size() != 1)
		throw Error("invalid SETUP DCS");

	if(in == -1 && (in = open(path.data(), O_WRONLY | O_NONBLOCK)) == -1)
	{
		//put("\021RET\022SETUP\0220\023"); this wont work
    	throw FatalError("failed to open input FIFO pipe");
    }

    return "\021RET\022SETUP\0221\023";
}

Glib::ustring Controller::dcs_move(Sequence seq)
{
	coord pos(0, 0);
	size_t endx = 0, endy = 0;

	if(seq.arg.size() != 4)
		throw Error("invalid arg");

	pos.x = std::stoll(seq.arg[2], &endx, 10);
	pos.y =	std::stoll(seq.arg[3], &endy, 10);

	if(endx != seq.arg[2].size() || endy != seq.arg[3].size() || pos.x < 0 || pos.y < 0)
		throw Error("invalid coord");

	if(seq.arg[1] == "OUT") outputer.move(pos);
	else if(seq.arg[1] == "IN") inputer.move(pos);
	else throw Error("invalid arg");

	return "\021RET\022MOVE\0221\023";
}

Glib::ustring Controller::dcs_find(Sequence seq)
{
	Glib::ustring ret("\021RET\022FIND\0221\022");
	coord pos(0, 0);

	if(seq.arg.size() != 2)
		throw Error("invalid arg");

	if(seq.arg[1] == "OUT")
		pos = outputer.find();
	else if(seq.arg[1] == "IN")
		pos = inputer.find();
	else
		throw Error("invalid arg");

	ret += std::to_string(pos.x) + "\022";
	ret += std::to_string(pos.y) + "\023";

	return ret;
}

Glib::ustring Controller::dcs_scroll(Sequence seq)
{
	coord pos(0, 0);
	size_t endx = 0, endy = 0;

	if(seq.arg.size() != 3)
		throw Error("invalid arg");

	pos.x = std::stoll(seq.arg[1]);
	pos.y = std::stoll(seq.arg[2]);

	if(endx != seq.arg[2].size() || endy != seq.arg[3].size() || pos.x < 0 || pos.y < 0)
		throw Error("invalid coord");

	scroll_pos = outputer.win.buf->get_iter_at_line_offset(pos.y, pos.x);

	Glib::signal_idle().connect(sigc::mem_fun(*this, &Controller::scroll), G_PRIORITY_LOW);

	return "\021RET\022SCROLL\0221\023";
}

Glib::ustring Controller::dcs_insert(Sequence seq)
{
	if(seq.arg.size() != 2)
		throw Error("invalid arg");

	outputer.insert(seq.arg[1]);

	return "\021RET\022INSERT\0221\023";
}

Glib::ustring Controller::dcs_edit(Sequence seq)
{
	if(seq.arg.size() != 2)
		throw Error("invalid arg");

	inputer.edit(seq.arg[1]);

	return "\021RET\022EDIT\0221\023";
}

Glib::ustring Controller::dcs_clear(Sequence seq)
{
	coord start(0, 0), end(0, 0);
	size_t last_start_x = 0, last_start_y = 0, last_end_x = 0, last_end_y = 0;

	if(seq.arg.size() == 5)
	{
		start.x = std::stoll(seq.arg[1], &last_start_x, 10);
		start.y = std::stoll(seq.arg[2], &last_start_y, 10);
		end.x = std::stoll(seq.arg[3], &last_end_x, 10);
		end.y = std::stoll(seq.arg[4], &last_end_y, 10);

		if
		(
			last_start_x != seq.arg[1].size() || last_start_y != seq.arg[2].size()
			|| last_end_x != seq.arg[3].size() || last_end_y != seq.arg[4].size()
			|| start.x < 0 || start.y < 0 || end.x < 0 || end.y < 0
		)
			throw Error("invalid coord");

		outputer.clear(start, end);
	}
	else if(seq.arg.size() == 1) outputer.clear();
	else throw Error("invalid arg");

	return "\021RET\022CLEAR\0221\023";
}

Glib::ustring Controller::dcs_embed(Sequence seq)
{
	if(seq.arg.size() != 2)
		throw Error("invalid arg");

	outputer.embed(seq.arg[1]);

	return "\021RET\022EMBED\0221\023";
}
