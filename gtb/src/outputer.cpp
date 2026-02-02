#include "gtb.h"

/*
*/

using namespace GTB;

Outputer::Outputer(Window& win_init) : win(win_init)
{
	if(pipe(out) == -1)
    	throw Error("GTB: failed to create output pipe");

	io_channel = Glib::IOChannel::create_from_fd(out[0]);

	io_channel->set_flags(Glib::IOFlags::IO_FLAG_NONBLOCK);

	Glib::RefPtr<Glib::IOSource> source = io_channel->create_watch(Glib::IOCondition::IO_IN | Glib::IOCondition::IO_PRI | Glib::IOCondition::IO_ERR | Glib::IOCondition::IO_HUP | Glib::IOCondition::IO_NVAL);

	source->connect(sigc::mem_fun(*this, &Outputer::outputer));
	source->set_priority(G_PRIORITY_HIGH); //Note using GTK C macro
	source->attach(Glib::MainContext::get_default());

	return;
}

Outputer::~Outputer()
{
	std::cout<<"~Outputer\n";

	if(cpid == CHLD_NULL && close(out[1]) == -1)
		std::cout<<"ERROR:GTB: failed to close output pipe\n";

	io_channel->close(true);

	return;
}

//This function is the signal handler attached to the IOChannel.
//This function is called every time data is available in the output pipe.
bool Outputer::outputer(const Glib::IOCondition cond)
{
	Glib::ustring text;
	Glib::IOStatus status;

	std::cout<<"\noutputer\n";

	if(cond & (Glib::IOCondition::IO_ERR | Glib::IOCondition::IO_NVAL))
	{
		std::cout<<"err\n";
		return false;
	}

	status = get_line(text);

	std::cout<<"text: "<<text;

	if(text.size()) overwrite(text);

	if(status == Glib::IOStatus::IO_STATUS_EOF && cond & Glib::IOCondition::IO_HUP)
	{
		std::cout<<"hup && eof\n";
		return false;
	}

	return true;
}

//This function reads a single line. Or if a complete line
//is unavailable it will return everything that is available
//up to a maximum of 1024 UTF codepoints.
Glib::IOStatus Outputer::get_line(Glib::ustring& str)
{
	gunichar c = 0, n = 1;
	gchar utf8_c[UTF8_CHAR_MAX + 1];
	Glib::IOStatus status;

	for(; c != '\n' && n <= 1024; n++)
	{
		status = io_channel->read(c);

		if(status != Glib::IOStatus::IO_STATUS_NORMAL)
			break;

		utf8_c[g_unichar_to_utf8(c, utf8_c)] = 0;

		str.append(utf8_c);
	}

	return status;
}

//This function displays a line without overwriting old
//output if present.
void Outputer::insert(Glib::ustring line)
{
	Glib::RefPtr<Gtk::TextMark> new_pos = win.buf->create_mark(win.outpos->get_iter(), false);
	Glib::RefPtr<Gtk::TextBuffer> tagged_line = apply_tags(line);

	win.buf->insert(win.outpos->get_iter(), tagged_line->begin(), tagged_line->end());

	win.buf->apply_tag_by_name("output", win.outpos->get_iter(), new_pos->get_iter());

	if(win.outpos->get_iter() == win.instart->get_iter())
		win.buf->move_mark(win.instart, new_pos->get_iter());

	win.buf->move_mark(win.outpos, new_pos->get_iter());

	win.buf->delete_mark(new_pos);

	Glib::signal_idle().connect(sigc::mem_fun(*this, &Outputer::scroll), G_PRIORITY_DEFAULT_IDLE);

	return;
}

//This function displays output and overwrites old output if it is present.
//This function will only overwrite output in the current line. It will
//not create a new line unless the output position is currently at the last line.
void Outputer::overwrite(Glib::ustring line)
{
	Glib::RefPtr<Gtk::TextBuffer> tagged_line = Gtk::TextBuffer::create(win.gtk_table);
	Glib::RefPtr<Gtk::TextMark> new_pos;
	Gtk::TextIter i, j, k;
	bool absorbed_nl = false, out_on_inend = false, out_on_cursor = false;

	if(!line.size()) return;

	std::cout<<"ow: "<<line;

	if(win.outpos->get_iter().get_line() + 1 != win.buf->get_line_count() && *line.rbegin() == '\n')
	{
		line = line.substr(0, line.size() - 1);
		absorbed_nl = true;
	}

	tagged_line->set_text(line);

	i = win.outpos->get_iter();
	k = tagged_line->begin();

	for(j = i; j != win.buf->end(); j++, k++)
	{
		if(*j == '\n' && j == win.instart->get_iter()) break;

		if(*k != '\n' && k == tagged_line->end()) break;
	}

	win.buf->erase(i, j);

	tagged_line = apply_tags(line);
	new_pos = win.buf->create_mark(win.outpos->get_iter(), false);

	if(win.outpos->get_iter() == win.inend->get_iter())
	{
		out_on_inend = true;

		if(win.outpos->get_iter() == win.buf->get_insert()->get_iter())
			out_on_cursor = true;
	}

	win.buf->insert(win.outpos->get_iter(), tagged_line->begin(), tagged_line->end());
	win.buf->apply_tag_by_name("output", win.outpos->get_iter(), new_pos->get_iter());

	//The order of these if statements is important
	//because both may be true.
	if(win.outpos->get_iter() == win.instart->get_iter())
		win.buf->move_mark(win.instart, new_pos->get_iter());
	else if(out_on_inend)
	{
		win.buf->move_mark(win.inend, win.outpos->get_iter());

		if(out_on_cursor) win.buf->place_cursor(win.outpos->get_iter());
	}

	if(absorbed_nl)
		win.buf->move_mark(win.outpos, win.buf->get_iter_at_line_offset(win.outpos->get_iter().get_line() + 1, 0));
	else
		win.buf->move_mark(win.outpos, new_pos->get_iter());

	win.buf->delete_mark(new_pos);

	Glib::signal_idle().connect(sigc::mem_fun(*this, &Outputer::scroll), G_PRIORITY_DEFAULT_IDLE);

	return;
}

bool Outputer::scroll()
{
	Gtk::TextIter iter = win.outpos->get_iter();

	win.view.scroll_to(iter);

	std::cout<<"scroll\n";

	return false;
}

//This function creates a GTK text buffer from the provided line. It then
//parses the text in the text buffer and applies any HTML tags it finds.
Glib::RefPtr<Gtk::TextBuffer> Outputer::apply_tags(Glib::ustring line)
{
	std::vector<Tag>::iterator i = win.table.begin();
	std::vector<TagArea>::iterator k;
	Gtk::TextIter j, match_start, match_end;
	Gtk::TextSearchFlags flags = ~(Gtk::TEXT_SEARCH_VISIBLE_ONLY | Gtk::TEXT_SEARCH_TEXT_ONLY | Gtk::TEXT_SEARCH_CASE_INSENSITIVE);

	Glib::RefPtr<Gtk::TextBuffer> tagged_line = Gtk::TextBuffer::create(win.gtk_table);

	tagged_line->set_text(line);

	std::cout<<"in apply_tags "<<open_tags.size()<<"\n";

	for(k = open_tags.begin(); k != open_tags.end(); k++)
		(*k).start = tagged_line->create_mark(tagged_line->begin(), true);

	for(; i != win.table.end(); i++)
	{
		for(j = tagged_line->begin(); j.forward_search((*i).start, flags, match_start, match_end); j = tagged_line->begin())
		{
			std::cout<<"found start tag\n";

			open_tags.push_back(TagArea(*i, tagged_line->create_mark(match_end, true)));

			tagged_line->erase(match_start, match_end);
		}

		for(j = tagged_line->begin(); j.forward_search((*i).end, flags, match_start, match_end); j = tagged_line->begin())
		{
			std::cout<<"found end tag\n";

			if((k = std::find(open_tags.begin(), open_tags.end(), *i)) != open_tags.end())
			{
				//std::cout<<"text = '"<<tagged_line->get_slice((*k).start->get_iter(), match_start, true)<<"'\n";

				tagged_line->apply_tag((*i).gtk_tag, (*k).start->get_iter(), match_start);
				tagged_line->erase(match_start, match_end);
				tagged_line->delete_mark((*k).start);

				open_tags.erase(k);
			}
			//else error
		}
	}

	//std::cout<<"start of split tags\n";

	for(k = open_tags.begin(); k != open_tags.end(); k++)
	{
		//std::cout<<"text = '"<<tagged_line->get_slice((*k).start->get_iter(), tagged_line->end(), true)<<"'\n";

		tagged_line->apply_tag((*k).tag.gtk_tag, (*k).start->get_iter(), tagged_line->end());
		tagged_line->delete_mark((*k).start);
	}

	std::cout<<"final text = "<<tagged_line->get_text()<<"\n";

	return tagged_line;
}

Outputer& Outputer::move(coord pos)
{
	Gtk::TextIter iter;

	std::cerr<<"start move\n";

	if(win.buf->get_line_count() < pos.y || win.buf->get_iter_at_line(pos.y).get_line_offset() < pos.x)
		throw Error("coordinate is unoccupied");

	iter = win.buf->get_iter_at_line_offset(pos.y, pos.x);

	if
	(
		win.instart->get_iter().get_line() == pos.y
		&&
		win.instart->get_iter().get_line_offset() < pos.x
		&&
		win.inend->get_iter().get_line_offset() > pos.x
	)
	{
		throw Error("coordinate inside input area");
	}

	win.buf->move_mark(win.outpos, iter);

	std::cerr<<"moved\n";

	return *this;
}

coord Outputer::find()
{
	coord pos(0, 0);

	pos.x = win.outpos->get_iter().get_line_offset();
	pos.y = win.outpos->get_iter().get_line();

	return pos;
}

Outputer& Outputer::clear()
{
	win.buf->erase(win.buf->begin(), win.buf->end());

	return *this;
}

Outputer& Outputer::clear(coord start, coord end)
{
	Gtk::TextIter start_iter, end_iter;

	start_iter = win.buf->get_iter_at_line_offset(start.y, start.x);
	end_iter = win.buf->get_iter_at_line_offset(end.y, end.x);

	win.buf->erase(start_iter, end_iter);

	return *this;
}

Outputer& Outputer::embed(Glib::ustring path)
{
	Glib::RefPtr<Gtk::TextMark> new_pos;
	Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_file(std::string(path));

	new_pos = win.buf->create_mark(win.outpos->get_iter(), false);

	win.buf->insert_pixbuf(win.outpos->get_iter(), pix);
	win.buf->apply_tag_by_name("output", win.outpos->get_iter(), new_pos->get_iter());

	if(win.outpos->get_iter() == win.instart->get_iter())
		win.buf->move_mark(win.instart, new_pos->get_iter());

	win.buf->move_mark(win.outpos, new_pos->get_iter());
	win.buf->delete_mark(new_pos);

	return *this;
}
