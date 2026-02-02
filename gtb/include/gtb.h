/*! \file gtb.h
    \brief Header file for Gnome TextBox
*/

/*! \mainpage Overview
	TextBox is a graphical application that provides all the functionality of a
	modern pseudo-terminal without the burden of emulating a terminal. TextBox also
	provides new functionality such as, event driven key binding, HTML like markup,
	embedded images, separate read and write positions, dynamic line oriented
	coordinates, and line editing semantics. Note you will need
	[libtb](https://codrod.github.io/libtb/index.html) to use a TextBox in your application.
	This implementation of TextBox uses the gtkmm bindings for the GIMP Toolkit (GTK).
*/

/*
	add middleware

	add DEBUG flag

	add unit testing where possible

	decide how to handle errors in outputer and inputer

	determine how to exit signal_handler thread properly
*/

#if !GTB_VERSION

	/* C */
	#include <signal.h>
	#include <errno.h>

	#include <unistd.h>
	#include <semaphore.h>
	#include <fcntl.h>
	#include <pthread.h>
	#include <sys/stat.h>

	#include <gtk/gtk.h>
	#include <gdk/gdk.h>
	#include <gdk/gdkkeysyms.h>

	/* C++ */
	#include <iostream>
	#include <string>
	#include <exception>
	#include <vector>
	#include <algorithm>
	#include <cstdlib>

	//debug
	#include <thread>
	#include <chrono>

	#include <glibmm/main.h>
	#include <glibmm/iochannel.h>
	#include <glibmm/dispatcher.h>

	#include <gtkmm/application.h>
	#include <gtkmm/applicationwindow.h>
	#include <gtkmm/scrolledwindow.h>
	#include <gtkmm/textview.h>
	#include <gtkmm/textbuffer.h>

	#define GTB_VERSION 1000000L

	#define DC1 '\021'
	#define DC2 '\022'
	#define DC3 '\023'

	//maximum UTF8 character size in bytes
	#define UTF8_CHAR_MAX 6

	#define CHLD_NULL -1
	#define CHLD_DEAD -2

	#define H1_SCALE_SIZE 2.0
	#define H2_SCALE_SIZE 1.5
	#define H3_SCALE_SIZE 1.17
	#define H4_SCALE_SIZE 1.0
	#define H5_SCALE_SIZE 0.83
	#define H6_SCALE_SIZE 0.67

	namespace GTB
	{
		class Error : public std::exception
		{
			public:
				std::string msg;

				Error(const std::string& str);
				Error(const char *str);

				virtual const char* what() const noexcept;
		};

		class FatalError : public std::exception
		{
			public:
				std::string msg;

				FatalError(const std::string& str);
				FatalError(const char *str);

				virtual const char* what() const noexcept;
		};

		class Sequence
		{
			public:
				//This is the DCS represented as an vector.
				//Each element is an argument of the DCS.
				std::vector<Glib::ustring> arg;

				//Parses the DCS in string form
				Sequence(Glib::ustring seq);
				Sequence();

				Sequence& parse(Glib::ustring seq);
		};

		class coord
		{
			public:
				long long int x;
				long long int y;

				coord(long long int x, long long int y);
				coord();
		};

		//Represents an HTML Tag
		class Tag
		{
			public:
				Glib::ustring start; //start tag i.e. <b>
				Glib::ustring end; //end tag i.e. </b>

				Glib::RefPtr<Gtk::TextTag> gtk_tag; //GTK tag

				Tag(Glib::RefPtr<Gtk::TextTag> gtk_tag, Glib::ustring start, Glib::ustring end);
				Tag();
		};

		//Represents the area in a tag i.e. <b>area</b>
		class TagArea
		{
			public:
				Tag tag;

				Glib::RefPtr<Gtk::TextMark> start; //represents position of the start tag

				TagArea(Tag tag, Glib::RefPtr<Gtk::TextMark> start);

				bool operator==(Tag tag);
		};

		//Represents the TextWindow. This class dosent
		//actually do anything it is just an object.
		class Window : public Gtk::ApplicationWindow
		{
			public:
				Gtk::ScrolledWindow scrolled_window;
				Gtk::TextView view;
				Glib::RefPtr<Gtk::TextBuffer> buf;
				Glib::RefPtr<Gtk::TextMark> outpos, instart, inend;
				Glib::Dispatcher dispatcher;

				std::vector<Tag> table; //HTML tag table
				Glib::RefPtr<Gtk::TextBuffer::TagTable> gtk_table; //GTK tag table

				Window();
				~Window();
		};

		//Outputer is responsible for displaying output on the 'Window'.
		//It contains the 'Window' object.
		class Outputer
		{
			public:
				Glib::RefPtr<Glib::IOChannel> io_channel; //this is used to integrate the
				Window& win;                         //pipe with the MainLoop in GTK
				int out[2]; //pipe for the output

				std::vector<TagArea> open_tags; //keeps track of open HTML tags

				Outputer(Window& win);
				~Outputer();

				bool outputer(const Glib::IOCondition cond);
				Glib::IOStatus get_line(Glib::ustring& text);

				void overwrite(Glib::ustring text);
				void insert(Glib::ustring text);
				bool scroll();
				Glib::RefPtr<Gtk::TextBuffer> apply_tags(Glib::ustring line);

				Outputer& move(coord pos);
				coord find();
				Outputer& clear();
				Outputer& clear(coord start, coord end);
				Outputer& embed(Glib::ustring path);

		};

		//Inputer is responsible for processing all user input and eventually
		//key bindings.
		class Inputer
		{
			public:
				Window& win;
				int in[2]; //input pipe

				Inputer(Window& win);
				~Inputer();

				bool inputer(GdkEventKey *event);

				Inputer& move(coord pos);
				coord find();

				Inputer& edit(Glib::ustring str);
		};

		//Controller is responsible for detecting and processing all device control sequences
		//sent from processes connected to GTB.
		class Controller
		{
			public:
				Outputer& outputer;
				Inputer& inputer;

				Glib::RefPtr<Glib::IOChannel> io_channel;
				int in; //input pipe
				int out; //output pipe
				Gtk::TextIter scroll_pos; //represents scroll position

				//This is used to synchronize access to the
				//input and output pipes.
				sem_t *lock;

				Controller(Inputer& inputer, Outputer& outputer);
				~Controller();

				bool controller(const Glib::IOCondition cond);
				Glib::IOStatus get(Glib::ustring& seq);
				int put(Glib::ustring seq);
				bool scroll();

				Glib::ustring exec(Sequence seq);
				Glib::ustring dcs_setup(Sequence seq);
				Glib::ustring dcs_move(Sequence seq);
				Glib::ustring dcs_find(Sequence seq);
				Glib::ustring dcs_scroll(Sequence seq);
				Glib::ustring dcs_insert(Sequence seq);
				Glib::ustring dcs_edit(Sequence seq);
				Glib::ustring dcs_clear(Sequence seq);
				Glib::ustring dcs_embed(Sequence seq);
		};

		extern pid_t pid; //GTB process ID
		extern pid_t cpid; //process ID of the GTB's immediate child

		//The single handling thread
		extern pthread_t signal_handler;

		extern Window *windowp;
		extern Outputer *outputerp;
		extern Inputer *inputerp;
		extern Controller *controllerp;

		void start_handler();
		void exit_handler();
		void* handler(void *arg);
	}

#endif
