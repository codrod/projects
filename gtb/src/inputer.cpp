#include "gtb.h"

using namespace GTB;

Inputer::Inputer(Window& win_init) : win(win_init)
{
	if(pipe(in) == -1)
    	throw Error("GTB: failed to create input pipe");

    win.signal_key_press_event().connect(sigc::mem_fun(*this, &Inputer::inputer), false);

    return;
}

Inputer::~Inputer()
{
	std::cout<<"~Inputer\n";

	if(cpid == CHLD_NULL && close(in[0]) == -1)
		std::cout<<"ERROR:GTB: failed to close input pipe\n";;

	if(close(in[1]) == -1)
		std::cout<<"ERROR:GTB: failed to close input pipe\n";

	return;
}

bool Inputer::inputer(GdkEventKey *event)
{
	Glib::ustring str;
	gunichar c = 0;
	gchar utf8[UTF8_CHAR_MAX + 1];
	Gtk::TextIter new_pos;

	if(Glib::ustring(gdk_keyval_name(event->keyval)) == "Return")
	{
		if(win.instart->get_iter().get_line() + 1 == win.buf->get_line_count())
		{
			win.buf->insert(win.inend->get_iter(), "\n");
			str = win.buf->get_text(win.instart->get_iter(), win.inend->get_iter(), false);

			new_pos = win.inend->get_iter();
		}
		else
		{
			str = win.buf->get_text(win.instart->get_iter(), win.inend->get_iter(), false);
			str.append("\n");

			new_pos = win.buf->get_iter_at_line_offset(win.instart->get_iter().get_line() + 1, 0);
		}

		win.buf->apply_tag_by_name("output", win.instart->get_iter(), win.inend->get_iter());

		if(win.outpos->get_iter() == win.instart->get_iter())
			win.buf->move_mark(win.outpos, new_pos);

		win.buf->move_mark(win.instart, new_pos);
		win.buf->move_mark(win.inend, new_pos);
		win.buf->place_cursor(new_pos);

		write(in[1], str.c_str(), str.size());

		return true;
	}
	//Note uses g_unichar_isprint() to ignore backspace, delete, escape, etc.
	else if(g_unichar_isprint((c = gdk_keyval_to_unicode(event->keyval))))
	{
		if
		(
			win.buf->get_insert()->get_iter() >= win.instart->get_iter()
			&&
			win.buf->get_insert()->get_iter() <= win.inend->get_iter()
		)
		{
			utf8[g_unichar_to_utf8(c, utf8)] = 0;

			//std::cout<<c<<"\n";

			win.buf->insert(win.buf->get_insert()->get_iter(), Glib::ustring(utf8));

			win.buf->remove_tag_by_name("output", win.instart->get_iter(), win.inend->get_iter());
		}

		return true;
	}

	return false;

}

Inputer& Inputer::move(coord pos)
{
	Gtk::TextIter new_pos, instart_iter, inend_iter;
	Glib::ustring input;

	new_pos = win.buf->get_iter_at_line_offset(pos.y, pos.x);

	instart_iter = win.instart->get_iter();
	inend_iter = win.inend->get_iter();

	win.buf->apply_tag_by_name("output", instart_iter, inend_iter);
	input = win.buf->get_slice(instart_iter, inend_iter);

	win.buf->move_mark(win.instart, new_pos);
	win.buf->place_cursor(new_pos);

	win.buf->erase(instart_iter, inend_iter);
	win.buf->insert(win.instart->get_iter(), input);

	inend_iter = win.buf->get_iter_at_offset(win.instart->get_iter().get_offset() + input.size());
	win.buf->move_mark(win.inend, inend_iter);

	win.buf->remove_tag_by_name("output", win.instart->get_iter(), inend_iter);

	return *this;
}

coord Inputer::find()
{
	coord pos(0, 0);

	pos.x = win.instart->get_iter().get_line_offset();
	pos.y = win.instart->get_iter().get_line();

	return pos;
}

Inputer& Inputer::edit(Glib::ustring str)
{
	win.buf->erase(win.instart->get_iter(), win.inend->get_iter());
	win.buf->insert(win.instart->get_iter(), str);

	win.buf->remove_tag_by_name("output", win.instart->get_iter(), win.inend->get_iter());

	return *this;
}
