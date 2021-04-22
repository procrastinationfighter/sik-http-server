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
static constexpr int BUFFER_SIZE = 1400;

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

void add_status_line(std::ostringstream &oss, int status_code) {
    oss << get_http_version_str() << " "
        << status_code << " "
        << get_reason_phrase(status_code) << get_CRLF();
}

void add_close_connection_header(std::ostringstream &oss) {
    oss << "Connection: close" << get_CRLF();
}

void add_content_length_header(std::ostringstream &oss, uintmax_t size) {
    oss << "Content-Length: " << size << get_CRLF();
}

void add_content_type_header(std::ostringstream &oss) {
    oss << "Content-Type: application/octet-stream" << get_CRLF();
}

void add_server_header(std::ostringstream &oss) {
    oss << "Server: ab417730_student_server" << get_CRLF();
}

void add_status_line_and_obligatory_headers(std::ostringstream &oss,
                                            int status_code) {
    add_status_line(oss, status_code);
    add_server_header(oss);
    if (status_code == RESPONSE_BAD_REQUEST
        || status_code == RESPONSE_NOT_IMPLEMENTED
        || status_code == RESPONSE_INTERNAL_ERROR) {
        add_close_connection_header(oss);
    }
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

Server::Server(std::string files_dir,
               std::string &&correlated_servers_file,
               int port)
    : files_dir(std::move(files_dir)),
      correlated_servers_file(std::move(correlated_servers_file)),
      sock(port, server_address) {}

void Server::send_start_line_and_headers(std::ostringstream &oss, FILE *output) {
    oss << get_CRLF();
    std::string first_part_of_response = oss.str();
    fputs(first_part_of_response.c_str(), output);
}

bool Server::is_file_in_directory(const std::string &file_path) const {
    // Checks if files directory is prefix of file_path.
    auto res = std::mismatch(files_dir.begin(), files_dir.end(), file_path.begin());
    return (res.first == files_dir.end())
        && (res.second != file_path.end())
        && (*res.second == '/');
}

void Server::send_response_with_file(const HttpRequest &request,
                                     FILE *output,
                                     const std::string &file_path_string) {
    std::ifstream file_stream;
    file_stream.open(file_path_string);

    if (!file_stream.is_open()) {
        throw FileOpeningError();
    }

    std::ostringstream oss;

    add_status_line_and_obligatory_headers(oss, RESPONSE_OK);
    add_content_length_header(oss, std::filesystem::file_size(file_path_string));
    if (request.should_close_connection()) {
        add_close_connection_header(oss);
    }
    send_start_line_and_headers(oss, output);

    // We send message body only if the requested method is GET.
    if (request.get_method() == HttpRequest::Method::GET) {
        // [TODO]: Handle error if sending failed.
        std::vector<char> buffer(BUFFER_SIZE);
        while (file_stream) {
            file_stream.read(buffer.data(), BUFFER_SIZE);
            std::streamsize s = file_stream ? BUFFER_SIZE : file_stream.gcount();

            fwrite(buffer.data(), sizeof(char), s, output);
        }
    }
    fflush(output);
}

void Server::handle_http_request(const HttpRequest &request, FILE *output) const {
    // Get path to the requested file.
    std::string file_path = files_dir + request.get_request_target();
    char *file_path_canon = canonicalize_file_name(file_path.c_str());
    if (file_path_canon == nullptr) {
        // [TODO]: File not found, check correlated.
    }

    std::string file_path_string(file_path_canon);
    free(file_path_canon);

    if (is_file_in_directory(file_path_string)) {
        try {
            send_response_with_file(request, output, file_path_string);
            return;
        } catch (FileOpeningError &e) {
            std::cout << "Local file could not be opened." << std::endl;
        }
    } else {
        std::cout << "File: " << file_path_string
                  << " not in directory: " << files_dir << std::endl;
    }

    // [TODO]: If we get here, we should check correlated files.
    std::cout << "Checking correlated files..." << std::endl;
}

void Server::send_fail_response(int status_code, FILE *output) {
    std::ostringstream oss;
    add_status_line_and_obligatory_headers(oss, status_code);
    send_start_line_and_headers(oss, output);
    fflush(output);
}

void Server::communicate_with_client(int msg_sock) {
    std::cout << "Communication with client started." << std::endl;

    FILE *input_file = fdopen(msg_sock, "r");
    FILE *output_file = fdopen(msg_sock, "w");

    // Caught exceptions are related with severing the connection.
    try {
        bool close_conn = false;
        while (!close_conn) {
            std::cout << "Dzialamy.\n";
            HttpRequest request = parse_http_request(input_file);
            handle_http_request(request, output_file);
            close_conn = request.should_close_connection();
        }
    } catch (IncorrectRequestFormat &e) {
        std::cerr << "Bad request: " << e.what() << std::endl;
        send_fail_response(RESPONSE_BAD_REQUEST, output_file);
    } catch (UnsupportedHttpMethod &e) {
        std::cerr << "Unsupported method: " << e.what() << std::endl;
        send_fail_response(RESPONSE_NOT_IMPLEMENTED, output_file);
    } catch (ServerInternalError &e) {
        std::cerr << "Internal error: " << e.what() << std::endl;
        send_fail_response(RESPONSE_INTERNAL_ERROR, output_file);
    } catch (ConnectionLost &e) {
        // Nothing else happens. Go handle the next client.
        std::cerr << "connection lost: " << e.what() << std::endl;
    } catch (std::exception &e) {
        // Again, nothing happens. Cut the connection.
        std::cerr << "Unexpected exception caught: " << e.what() << std::endl;
    }

    std::cout << "Communication with client finished." << std::endl;
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
    if (normalized_dir == nullptr) {
        syserr("Given directory with files not found.");
    }
    std::string files_dir(normalized_dir);
    char *normalized_file = canonicalize_file_name(argv[2]);

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