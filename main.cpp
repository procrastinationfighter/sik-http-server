#include <iostream>
#include <regex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>

constexpr int DEFAULT_PORT = 8080;

const std::string &get_http_version_str() {
    static const std::string http_version = "HTTP/1.1";
    return http_version;
}

const std::string &get_CRLF() {
    static const std::string CRLF = "\r\n";
    return CRLF;
}

const std::string &get_methods_str() {
    static const std::string methods = "(?:HEAD|GET)";
    return methods;
}

const std::string &get_request_target_str() {
    static const std::string str = R"((?:/|(?:/[a-zA-Z0-9.-]+)+))";
    return str;
}

const std::string &get_request_line_str() {
    static const std::string line = R"(\w+ )"
        + get_request_target_str()
        + " "
        + get_http_version_str()
        + get_CRLF();
    return line;
}

const std::string &get_field_value_str() {
    static const std::string field = R"(\w+)";
    return field;
}

const std::string &get_header_field_str() {
    static const std::string header = R"(\w+:\s*)"
        + get_field_value_str()
        + R"(\s*)";
    return header;
}

const std::string &get_mess_body_str() {
    // Since our server accepts only GET and HEAD methods,
    // body should be empty.
    static const std::string body;
    return body;
}

const std::string &get_mess_request_str() {
    static const std::string message = "(" + get_request_line_str() + ")"
        + "(" + get_header_field_str() + get_CRLF() + ")*"
        + get_CRLF()
        + "(" + get_mess_body_str() + ")";
    return message;
}

const std::regex &get_directory_regex() {
    static const std::regex regex(R"(/?(\w+/)*)");
    return regex;
}

void exit_fail() {
    exit(EXIT_FAILURE);
}

class Server {
  private:
    std::string files_directory;
    std::string correlated_servers_file;

    int sock, msg_sock;
    struct sockaddr_in server_address;

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
            std::cerr << "Socket error.\n";
            exit_fail();
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
            std::cerr << "Bind error.\n";
            exit_fail();
        }
    }

    static void check_arg_count(int argc) {
        if (argc < 3 || argc > 5) {
            std::cerr << "ERROR:\n"
                      << "Program correct usage: "
                      << "<PROGRAM> <SERVER_FILES_DIRECTORY> <CORRELATED_SERVERS_FILE> [OPT]<PORT>\n";
            exit_fail();
        }
    }

    static int parse_port_argument(const std::string &argv) {
        try {
            size_t len;
            int port = std::stoi(argv, &len);
            if (len == argv.length() && port <= MAXIMAL_PORT_NUMBER && port >= MINIMAL_PORT_NUMBER) {
                return port;
            } else {
                throw std::invalid_argument("stoi");
            }
        } catch (std::exception &e) {
            std::cerr << "ERROR:\n"
                      << "Given port is not valid.\n";
            exit_fail();
        }

        return 0;
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
            std::cerr << "Listen error.\n";
            exit_fail();
        }
        struct sockaddr_in client_address;
        socklen_t client_address_len;
        char buffer[BUFFER_SIZE];
        ssize_t len, snd_len;

        for (;;) {
            client_address_len = sizeof(client_address);
            // get client connection from the socket
            msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
            do {
                len = read(msg_sock, buffer, sizeof(buffer));
                printf("read from socket: %zd bytes: %.*s\n", len, (int) len, buffer);
            } while (len > 0);
            printf("ending connection\n");
            close(msg_sock);
        }
    }
};

int main(int argc, char *argv[]) {
    // [TODO] 1. Check arguments. (first: directory, second: file, third (optional): port)
    //        2. Start server and wait for clients.
    //        3. Parse queries.
    Server server = Server::create_from_program_arguments(argc, argv);
    server.run();

    return 0;
}
