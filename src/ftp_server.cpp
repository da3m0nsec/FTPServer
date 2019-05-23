#include <iostream>
#include <memory>
#include <signal.h>

#include "FTPServer.h"

std::unique_ptr<FTPServer> server;

void exit_handler();
extern "C" void int_signal_handler(int signum, siginfo_t*, void*);


int main(int argc, char **argv)
{
    struct sigaction action{};
    action.sa_sigaction = int_signal_handler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &action , nullptr);

    server = std::make_unique<FTPServer>(FTPServer());
    atexit(exit_handler);
    server->run();

    return 0;
}


void exit_handler()
{
    server->stop();
    exit(-1);
}

extern "C" void int_signal_handler(int signum, siginfo_t*, void*)
{
    std::cout << "\nInterrupt signum (" << signum << ") received." << std::endl;
    exit_handler();
}

