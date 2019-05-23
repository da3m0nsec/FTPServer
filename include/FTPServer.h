#pragma once

#include <list>

#include "ClientConnection.h"

/**
 *
 * @class FTPServer
 * @brief File Transfer Protocol Server Implementation
 *
 * @details This class manages the set up of the FTP server
 * @note The 2121 port will be used as default.
 *
 * @author Javier Alonso Delgado
 * @author Alvaro Fernandez Rodriguez
 *
 * @date 2019/05/23
 *
*/
class FTPServer
{
    private:

        int port_;  /**< Server port. */
        int msock_; /**< Socket descriptor. */

        std::list<ClientConnection*> connection_list_;  /**< List of the connected clients. */

    public:

        /**
         * @brief Initialize the port value
         * @param port -> 2121 as default value
         */
        explicit FTPServer(int port = 2121);

        /**
         * @brief Start up the server
         */
        void run();
        /**
         * @brief Shutdown the server
         */
        void stop();
};

