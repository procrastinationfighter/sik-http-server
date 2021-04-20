#ifndef SIK1__SERVER_H_
#define SIK1__SERVER_H_

#include <iostream>
#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include "server_utilities.h"


class Server {
  private:
    std::string files_directory;
    std::string correlated_servers_file;

    int sock;
    struct sockaddr_in server_address;

    static constexpr int DEFAULT_PORT = 8080;
    static constexpr int MAXIMAL_PORT_NUMBER = 65535;
    static constexpr int MINIMAL_PORT_NUMBER = 1024;
    static constexpr int QUEUE_LENGTH = 5;
    static constexpr int BUFFER_SIZE = 1500;

    Server(std::string &&files_dir,
           std::string &&correlated_servers_file,
           int port)
        : files_directory(std::move(files_dir)),
          correlated_servers_file(std::move(correlated_servers_file)) {;
        if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
           syserr("Socket\n");
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
            syserr("Bind\n");
            exit_fail();
        }
    }

    static void check_arg_count(int argc) {
        if (argc < 3 || argc > 5) {
          syserr("Program correct usage: <PROGRAM> <SERVER_FILES_DIRECTORY> <CORRELATED_SERVERS_FILE> [OPT]<PORT>\n");
        }
    }

    static int parse_port_argument(const std::string &argv) {
        try {
            size_t len;
            int port = std::stoi(argv, &len);
            if (len == argv.length() && port <= MAXIMAL_PORT_NUMBER && port >= MINIMAL_PORT_NUMBER) {
                return port;
            } else {
                throw std::invalid_argument("wrong port");
            }
        } catch (std::invalid_argument &e) {
            syserr("Given port is not valid.\n");
        }

        return -1;
    }

    void communicate_with_client(int msg_sock) const {
        ssize_t len = 0;
        char buffer[BUFFER_SIZE];
        do {
            len = read(msg_sock, buffer, sizeof(buffer));
            if (len < 0) {
                syserr("Reading from client socket.\n");
            }
            printf("read from socket: %zd bytes: %.*s\n", len, (int) len, buffer);
        } while (len > 0);
    }

    void set_communicaion_with_client() const {
        int msg_sock;
        struct sockaddr_in client_address;
        socklen_t client_address_len;

        client_address_len = sizeof(client_address);
        // get client connection from the socket
        msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (msg_sock < 0) {
            syserr("Accept\n");
        }


        communicate_with_client(msg_sock);

        if (close(msg_sock) < 0) {
            syserr("Close\n");
        }
    }

  public:
    static Server create_from_program_arguments(int argc, char *argv[]) {
        check_arg_count(argc);

        // [TODO]: Check other arguments.

        int port_number = (argc == 4 ? parse_port_argument(argv[3]) : DEFAULT_PORT);

        return Server(std::string(argv[1]),
                      std::string(argv[2]),
                      port_number);
    }

    [[noreturn]] void run() {
        if (listen(sock, QUEUE_LENGTH) < 0) {
            syserr("Listen\n");
        }

        for (;;) {
            set_communicaion_with_client();
        }
    }
};

#endif //SIK1__SERVER_H_
