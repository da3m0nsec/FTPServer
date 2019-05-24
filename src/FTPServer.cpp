#include <cerrno>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <list>
#include <system_error>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <pthread.h>
#include <unistd.h>

#include "ClientConnection.h"
#include "FTPServer.h"


int define_socket_TCP(int port)
{
    struct sockaddr_in address{};
    int socketFd;

    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if(socketFd < 0)
    {
        throw std::system_error(errno, std::system_category(), "Cannot create socket");
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(socketFd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Cannot create socket");
    }

    if(listen(socketFd, 128) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Cannot listen");
    }

    return socketFd;
}

void* run_client_connection(void *c)
{
    auto connection = static_cast<ClientConnection*>(c);
    connection->WaitForRequests();
  
    return nullptr;
}

FTPServer::FTPServer(int port)
    : msock_(), port_(port)
{}

void FTPServer::stop()
{
    close(msock_);
    shutdown(msock_, SHUT_RDWR);
}

void FTPServer::run()
{
    struct sockaddr_in fsin{};
    int ssock;

    socklen_t alen = sizeof(fsin);
    msock_ = define_socket_TCP(port_);

    while(true)
    {
	    pthread_t thread;
        ssock = accept(msock_, reinterpret_cast<sockaddr*>(&fsin), &alen);

        if(ssock < 0)
        {
            throw std::system_error(errno, std::system_category(), "Cannot accept");
        }

	    auto *connection = new ClientConnection(ssock);

	    pthread_create(&thread, nullptr, run_client_connection, (void*)connection);
    }

}
