#include "gtb.h"

using namespace GTB;

Tag::Tag(Glib::RefPtr<Gtk::TextTag> gtk_tag_init, Glib::ustring start_init, Glib::ustring end_init)
{
	gtk_tag = gtk_tag_init;
	start = start_init;
	end = end_init;

	return;
}

Tag::Tag()
{
	return;
}

TagArea::TagArea(Tag tag_init, Glib::RefPtr<Gtk::TextMark> start_init)
{
	tag = tag_init;
	start = start_init;

	return;
}

bool TagArea::operator==(Tag other_tag)
{
	//std::cout<<"== "<<(tag.start == other_tag.start)<<"\n";

	return tag.start == other_tag.start;
}
