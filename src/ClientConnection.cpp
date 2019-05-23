#include <array>
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cerrno>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <langinfo.h>
#include <locale.h>
#include <netdb.h>
#include <pwd.h>
#include <system_error>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "ClientConnection.h"

#define COMMAND(cmd) strcmp(command_, cmd) == 0


ClientConnection::ClientConnection(int s)
    : stop_(false)
    , fd_(nullptr)
    , command_()
    , arg_()
{
    control_socket_ = s;
    fd_ = fdopen(s, "a+");

    if (fd_ == nullptr)
    {
        std::cout << "Connection closed" << std::endl;

        fclose(fd_);
        close(control_socket_);
        ok_ = false;
        return;
    }
    
    ok_ = true;
    data_socket_ = -1;
}

ClientConnection::~ClientConnection()
{
 	fclose(fd_);
	close(control_socket_);
}


int connect_TCP(uint32_t ip_address, uint16_t port)
{
    struct sockaddr_in address{};

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = ip_address;
    address.sin_port = port;

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if(socketFd < 0)
    {
        throw std::system_error(errno, std::system_category(), "Cannot create socket");
    }

    if(connect(socketFd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0)
    {
        throw std::system_error(errno, std::system_category(), "Cannot connect to server");
    }

    return socketFd;
}

bool userExists (char* username){
    std::ifstream file("../database/userlist.txt");
    std::string tmp;
    if(file.good()){
        while(!file.eof()){ 
            file >> tmp;
            if (strcmp(tmp.c_str(), username) == 0){
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}


void ClientConnection::stop()
{
    close(data_socket_);
    close(control_socket_);
    stop_ = true;
}


void ClientConnection::WaitForRequests()
{
    if (!ok_)
    {
        return;
    }

    namespace fs = std::experimental::filesystem;
    fs::path current_dir(fs::current_path());
    
    fprintf(fd_, "220 Service ready\n");
  
    while(!stop_)
    {
        fscanf(fd_, "%s", command_);

        if (COMMAND("USER"))
        {
            //fscanf(fd_, "%s", arg_);
            if(fscanf(fd_, "%s", arg_) == 1)
            {
                //if(strcmp("anon", arg_) == 0)
                if(userExists(arg_)){
                    fprintf(fd_, "331 User name ok, need password\n");
                    fflush(fd_);
                }
                else{
                    fprintf(fd_, "430 Invalid username or password\n");
                    fflush(fd_);
                }
            }   
            else
            {
                fprintf(fd_, "501 Syntax error in parameters or arguments.\n");
                fflush(fd_);
            }

        }
        else if (COMMAND("PWD"))
        {
            fprintf(fd_, "257 \"%s\" is current directory.\n", current_dir.c_str());
        }
        else if (COMMAND("PASS"))
        {
            //fscanf(fd_, "%s", arg_);
            if(fscanf(fd_, "%s", arg_) == 1)
            {
                if(strcmp("passwd", arg_) == 0){
                    fprintf(fd_, "230 User logged in\n");
                }
                else{
                    fprintf(fd_, "430 Invalid username or password\n");
                    fflush(fd_);
                }
            } 
        }
        else if (COMMAND("PORT"))
        {
            unsigned int h0, h1, h2, h3;
            unsigned int p0, p1;

            fscanf(fd_, "%u, %u, %u, %u, %u, %u", &h0, &h1, &h2, &h3, &p0, &p1);

            uint32_t address = h3 << 24u | h2 << 16u | h1 << 8u | h0;
            uint16_t port = p1 << 8u | p0;
            data_socket_ = connect_TCP(address, port);

            if(data_socket_ < 0)
            {
                fprintf(fd_, "421 Service not available, closing control connection.\n");
            }
            else
            {
                fprintf(fd_, "200 OK\n");
            }
        }
        else if (COMMAND("PASV"))
        {
            int socketFd = socket(AF_INET,SOCK_STREAM, 0);

            if (socketFd < 0)
            {
                throw std::system_error(errno, std::system_category(), "Cannot create socket");
            }

            struct sockaddr_in input_addr{};
            memset(&input_addr, 0, sizeof(input_addr));
            input_addr.sin_family = AF_INET;
            inet_pton(AF_INET, "127.0.0.1", &input_addr.sin_addr.s_addr);
            input_addr.sin_port = 0;

            struct sockaddr_in output_addr{};
            socklen_t od_len = sizeof(output_addr);

            if(bind(socketFd, reinterpret_cast<sockaddr*>(&input_addr), sizeof(input_addr)) < 0)
            {
                throw std::system_error(errno, std::system_category(), "Cannot bind socket");
            }
            if (listen(socketFd, 128) < 0)
            {
                throw std::system_error(errno, std::system_category(), "Cannot listen to socket");
            }

            getsockname(socketFd, reinterpret_cast<sockaddr*>(&output_addr), &od_len);

            unsigned short p0 = (output_addr.sin_port & 0xff), p1 = (output_addr.sin_port >> 8);

            fprintf(fd_, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u).\n", 127u, 0u, 0u, 1u, p0, p1);
            fflush(fd_);
            
            data_socket_ = accept(socketFd,(struct sockaddr *)&output_addr, &od_len);
        }
        else if (COMMAND("CWD"))
        {
            fscanf(fd_, "%s", arg_);
            std::string new_dir_name(arg_);
            fs::path new_dir(new_dir_name);

            if (fs::exists(new_dir) && fs::is_directory(new_dir))
            {
                current_dir = new_dir;
                fprintf(fd_, "200 directory changed to \"%s\"\n", current_dir.c_str());
            }
            else
            {
                fprintf(fd_, "431 no such directory\n");
            }
        }
        else if (COMMAND("STOR"))
        {
            fscanf(fd_, "%s", arg_);

            std::array<char, 1024> buffer;
            std::ofstream file(current_dir.string() + '/' + arg_);
            int bytes;

            if(file.good())
            {
                fprintf(fd_, "150 File status okay; about to open data connection.\n");
                fflush(fd_);
                do
                {
                    bytes = recv(data_socket_, buffer.data(), 1024, 0);
                    file.write(buffer.data(), bytes);
                }
                while(bytes == 1024);

                fprintf(fd_, "226 Closing data connection.\n");
                file.close();
            }
            else
            {
                fprintf(fd_, "552 Requested file action aborted.\n");
            }
            close(data_socket_);
        }
        else if (COMMAND("SYST"))
        {
            fprintf(fd_, "215 UNIX Type: L8.\n");
        }
        else if (COMMAND("TYPE"))
        {
            fscanf(fd_, "%s", arg_);
            fprintf(fd_, "200 OK\n");
        }
        else if (COMMAND("RETR"))
        {
            fscanf(fd_, "%s", arg_);

            std::array<char, 1024> buffer;
            std::ifstream file(current_dir.string() + '/' + arg_);

            if(file.good())
            {
                fprintf(fd_, "150 File status okay; about to open data connection.\n");
                do
                {
                    file.read(buffer.data(), 1024);
                    send(data_socket_, buffer.data(), file.gcount(), 0);
                }
                while(file.gcount() == 1024);

                fprintf(fd_, "226 Closing data connection.\n");
                file.close();
            }
            else
            {
                fprintf(fd_, "550 Requested action not taken.\n");
            }
            close(data_socket_);
        }
        else if (COMMAND("QUIT"))
        {
            fprintf(fd_, "221 Service closing control connection.\n");
            fflush(fd_);
            stop();
        }
        else if (COMMAND("LIST"))
        {
            fprintf(fd_, "125 List started OK.\n");
            for(auto& entry: fs::directory_iterator(current_dir))
            {
                const auto filename = entry.path().filename().string() + '\n';
                send(data_socket_, filename.c_str(), filename.size(), 0);
            }
            fprintf(fd_, "250 List completed successfully.\n");
            close(data_socket_);
        }
        else
        {
            fprintf(fd_, "502 Command not implemented.\n");
            fflush(fd_);
            printf("Command: %s %s\n", command_, arg_);
            printf("Internal server error\n");
        }
    }
    fclose(fd_);
}


