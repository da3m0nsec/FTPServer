set(TARGET target_name)
add_executable(${TARGET} ClientConnection.cpp ftp_server.cpp FTPServer.cpp)
