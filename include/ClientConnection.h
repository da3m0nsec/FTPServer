#pragma once

#include <pthread.h>

const int MAX_BUFF = 1000;

/**
 *
 * @class ClientConnection
 * @brief FTP Client class
 *
 * @details This class manages the connection with the client and
 * the execution of commands sent by him.
 *
 * @author Javier Alonso Delgado
 * @author Alvaro Fernandez Rodriguez
 *
 * @date 2019/05/23
 *
*/
class ClientConnection
{
    private:

        bool ok_;
        bool stop_;

        FILE* fd_;                /**< File descriptor for sending control messages. */

        char command_[MAX_BUFF];  /**< Buffer for saving the command. */
        char arg_[MAX_BUFF];      /**< Buffer for saving the arguments. */

        int data_socket_;         /**< Data socket descriptor. */
        int control_socket_;      /**< Control socket descriptor. */

    public:

        /**
         * @brief Setups the socket descriptors
         * @param s -> descriptor for the control socket
         */
        explicit ClientConnection(int s);
        /**
         * @brief Close the file descriptor and socket
         */
        ~ClientConnection();

        /**
         * @brief Interprets and executes FTP commands
         */
        void WaitForRequests();
        /**
         * @brief Shutdown the connection
         */
        void stop();
};

