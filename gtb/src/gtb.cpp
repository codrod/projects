#include "gtb.h"

using namespace GTB;

pid_t GTB::pid;
pid_t GTB::cpid;

pthread_t GTB::signal_handler;

GTB::Window *GTB::windowp;
GTB::Outputer *GTB::outputerp;
GTB::Inputer *GTB::inputerp;
GTB::Controller *GTB::controllerp;

int main(int argc, char* *argv)
{
	try
	{
		sigset_t set;
		sigfillset(&set);

		if(pthread_sigmask(SIG_BLOCK, &set, NULL))
			throw Error("GTB: failed to disable POSIX signal handlers");

		int ret = 0;
		GTB::pid = getpid();
		GTB::cpid = CHLD_NULL;

		if(setenv("TW_PID", std::to_string(GTB::pid).data(), 0) == -1)
			throw Error("GTB: failed to set TW_PID environmental variable");

		Glib::RefPtr<Gtk::Application> app = Gtk::Application::create("org.GTB-99");
		Window window;
		Outputer outputer(window);
		Inputer inputer(window);
		Controller controller(inputer, outputer);

		GTB::windowp = &window;
		GTB::outputerp = &outputer;
		GTB::inputerp = &inputer;
		GTB::controllerp = &controller;

		if(argc > 1)
		{
			if(!(ret = fork()))
			{
				if(close(outputer.out[0]) == -1)
					throw Error("GTB: failed to open output pipe");

				if(close(inputer.in[1]) == -1)
					throw Error("GTB: failed to open input pipe");

				if(dup2(outputer.out[1], STDOUT_FILENO) == -1)
					throw Error("GTB: failed to dup stdout");

				if(dup2(inputer.in[0], STDIN_FILENO) == -1)
					throw Error("GTB: failed to dup stdin");

				execvp(argv[1], &argv[1]);

				std::cout<<"ERROR:GTB: failed to start child process \""<<argv[1]<<"\" : "<<strerror(errno)<<"\n";

				exit(1);
			}
			else
			{
				if(ret == -1)
					throw Error("GTB: failed to fork child process");

				GTB::cpid = ret;

				if(close(outputer.out[1]) == -1)
					throw Error("GTB: failed to open output pipe");

				if(close(inputer.in[0]) == -1)
					throw Error("GTB: failed to open input pipe");
			}
		}

		app->signal_startup().connect(sigc::ptr_fun(&GTB::start_handler));
		app->signal_shutdown().connect(sigc::ptr_fun(&GTB::exit_handler));

		ret = app->run(window);

		if(GTB::cpid != CHLD_DEAD && GTB::cpid != CHLD_NULL && kill(GTB::cpid, SIGTERM) == -1) //note: only kills immediate child
			throw Error("GTB: failed to kill child process");

		return ret;
	}
	//Consider checking for other common exceptions like bad_alloc
	catch(Error ex)
	{
		std::cout<<"ERROR:"<<ex.what()<<"\n";
		return 1;
	}
	catch(...)
	{
		std::cout<<"ERROR:GTB: unknown error\n";
		return 1;
	}
}

void GTB::start_handler()
{
	pthread_create(&GTB::signal_handler, NULL, &handler, NULL);

	return;
}

void GTB::exit_handler()
{
	pthread_cancel(GTB::signal_handler);

	return;
}

void* GTB::handler(void *arg)
{
	sigset_t set;
	siginfo_t info;

	sigfillset(&set);

	while(sigwaitinfo(&set, &info) != -1)
	{
		if(info.si_signo == SIGCHLD)
		{
			if(info.si_code == CLD_EXITED || info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED)
				GTB::cpid = CHLD_DEAD;
		}
		else
		{
			std::cout<<"GTB:STATUS: terminate signal received\n";

			windowp->dispatcher.emit();

			break;
		}
	}

	if(errno) windowp->dispatcher.emit();

	return NULL;
}
