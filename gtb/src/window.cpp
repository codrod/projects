#include "gtb.h"

using namespace GTB;

Window::Window() : Gtk::ApplicationWindow()
{
	Glib::RefPtr<Gtk::TextTag> gtk_tag;

	this->set_title("GTB-99");
    this->set_position(Gtk::WindowPosition::WIN_POS_CENTER);
    this->set_default_size(650, 350);
    this->add_events(Gdk::EventMask::KEY_PRESS_MASK);

    scrolled_window.set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_ALWAYS);

    view.set_wrap_mode(Gtk::WrapMode::WRAP_CHAR);
    view.set_left_margin(10);
    view.set_right_margin(10);

    gtk_table = Gtk::TextBuffer::TagTable::create();
    buf = Gtk::TextBuffer::create(gtk_table);
    view.set_buffer(buf);

    gtk_tag = buf->create_tag();
    gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
	table.push_back(Tag(gtk_tag, "<b>", "</b>"));

	gtk_tag = buf->create_tag();
    gtk_tag->property_style() = Pango::Style::STYLE_ITALIC;
	table.push_back(Tag(gtk_tag, "<i>", "</i>"));

	gtk_tag = buf->create_tag();
    gtk_tag->property_underline() = Pango::Underline::UNDERLINE_SINGLE;
	table.push_back(Tag(gtk_tag, "<u>", "</u>"));

	gtk_tag = buf->create_tag();
    gtk_tag->property_justification() = Gtk::Justification::JUSTIFY_CENTER;
	table.push_back(Tag(gtk_tag, "<center>", "</center>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H1_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h1>", "</h1>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H2_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h2>", "</h2>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H3_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h3>", "</h3>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H4_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h4>", "</h4>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H5_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h5>", "</h5>"));

	gtk_tag = buf->create_tag();
	gtk_tag->property_weight() = PANGO_WEIGHT_BOLD;
    gtk_tag->property_scale() = H6_SCALE_SIZE;
	table.push_back(Tag(gtk_tag, "<h6>", "</h6>"));

    buf->create_tag("output")->property_editable() = false;

    outpos = buf->create_mark("outpos", buf->begin(), true);
   	instart = buf->create_mark("instart", buf->begin(), true);
    inend = buf->create_mark("inend", buf->begin(), false);
    outpos->set_visible(true);
    instart->set_visible(true);
    inend->set_visible(true);

    dispatcher.connect(sigc::mem_fun(*this, &Window::close));

    this->add(scrolled_window);
    scrolled_window.add(view);
    show_all_children();

    return;
}

Window::~Window()
{
	std::cout<<"~Window\n";

	return;
}
