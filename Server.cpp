#include "Server.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <fstream>

static constexpr int DEFAULT_PORT = 8080;
static constexpr int MAXIMAL_PORT_NUMBER = 65535;
static constexpr int MINIMAL_PORT_NUMBER = 1024;
static constexpr int QUEUE_LENGTH = 5;
static constexpr int BUFFER_SIZE = 1500;

namespace {
void check_arg_count(int argc) {
    if (argc < 3 || argc > 5) {
        syserr(
            "Program correct usage: <PROGRAM> <SERVER_FILES_DIRECTORY> <CORRELATED_SERVERS_FILE> [OPT]<PORT>\n");
    }
}

int parse_port_argument(const std::string &argv) {
    try {
        size_t len;
        int port = std::stoi(argv, &len);
        if (len == argv.length() && port <= MAXIMAL_PORT_NUMBER && port >= MINIMAL_PORT_NUMBER) {
            return port;
        } else {
            throw std::invalid_argument("wrong port");
        }
    } catch (std::invalid_argument &e) {
        syserr("Given port is not valid.");
    }

    return -1;
}
}

SocketWrapper::SocketWrapper(int descriptor) : descriptor(descriptor) {}

SocketWrapper::SocketWrapper(int port, struct sockaddr_in &server_address) {
    if ((descriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        syserr("Socket");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(descriptor, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        syserr("Bind");
    }
}

SocketWrapper::~SocketWrapper() {
    if (close(descriptor) < 0) {
        syserr("Close");
    }
}

int SocketWrapper::get_descriptor() const {
    return descriptor;
}

Server::Server(std::string &&files_dir,
               std::string &&correlated_servers_file,
               int port)
    : files_dir(std::move(files_dir)),
      correlated_servers_file(std::move(correlated_servers_file)),
      sock(port, server_address) {}

bool Server::is_file_in_directory(const std::string &file_path) const {
    // Checks if files directory is prefix of file_path.
    auto res = std::mismatch(files_dir.begin(), files_dir.end(), file_path.begin());
    return res.first == files_dir.end();
}

void Server::handle_http_request(const HttpRequest &request, FILE *output) const {
    // Get path to the requested file.
    std::string file_path = files_dir + request.get_request_target();
    file_path = canonicalize_file_name(file_path.c_str());

    if (is_file_in_directory(file_path)) {
        std::ifstream file_stream;
        file_stream.open(file_path);
        if (file_stream.is_open()) {
            // [TODO]: Send file.
        } else {
            // [TODO]: Check in correlated files.
        }
    } else {
        // [TODO]: Send 404
    }
}

void Server::communicate_with_client(int msg_sock) {
    ssize_t len = 0;
    char buffer[BUFFER_SIZE];

    std::cout << "Communication with client started.\n";

    FILE *input_file = fdopen(msg_sock, "r");
    FILE *output_file = fdopen(msg_sock, "w");

    try {
        bool close_conn = false;
        while (!close_conn) {
            HttpRequest request = parse_http_request(input_file);
            handle_http_request(request, output_file);
            close_conn = request.should_close_connection();
        }
    } catch (ConnectionLost &e) {
        // [TODO]: Write error code.
        syserr("connection lost: " << e.what());
    } catch (IncorrectRequestFormat &e) {
        syserr("wrong request: " << e.what());
    } catch (UnsupportedHttpMethod &e) {
        syserr("unsupported method: " << e.what());
    } catch (ServerInternalError &e) {
        syserr("internal error: " << e.what());
    } catch (std::exception &e) {
        // [TODO]: Handle dis.
        syserr("Unexpected exception caught");
    }

    std::cout << "Communication with client finished.\n";
}

void Server::set_communicaion_with_client() {
    int msg_sock;
    sockaddr_in client_address;
    socklen_t client_address_len;

    client_address_len = sizeof(client_address);
    msg_sock = accept(sock.get_descriptor(), (sockaddr *) &client_address, &client_address_len);
    if (msg_sock < 0) {
        syserr("Accept");
    }

    SocketWrapper msg_socket_wrapper(msg_sock);

    communicate_with_client(msg_sock);
}

Server Server::create_from_program_arguments(int argc, char *argv[]) {
    check_arg_count(argc);

    char *normalized_dir = canonicalize_file_name(argv[1]);
    char *normalized_file = canonicalize_file_name(argv[2]);

    std::filesystem::path files_dir(normalized_dir);

    int port_number = (argc == 4 ? parse_port_argument(argv[3]) : DEFAULT_PORT);

    free(normalized_dir);
    free(normalized_file);
    return Server(files_dir,
                  std::string(argv[2]),
                  port_number);
}

[[noreturn]] void Server::run() {
    if (listen(sock.get_descriptor(), QUEUE_LENGTH) < 0) {
        syserr("Listen\n");
    }

    std::cout << "Started listening.\n";

    for (;;) {
        set_communicaion_with_client();
    }
}