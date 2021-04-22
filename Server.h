#ifndef SIK1__SERVER_H_
#define SIK1__SERVER_H_

#include <netinet/in.h>
#include <filesystem>
#include "server_utilities.h"
#include "HttpRequest.h"

class SocketWrapper {
  private:
    int descriptor;
  public:
    explicit SocketWrapper(int descriptor);
    SocketWrapper(int port, sockaddr_in &server_address);
    ~SocketWrapper();
    [[nodiscard]] int get_descriptor() const;
};

class Server {
  private:
    std::string files_dir;
    std::string correlated_servers_file;

    SocketWrapper sock;
    struct sockaddr_in server_address;

    Server(std::string files_dir, std::string &&correlated_servers_file, int port);

    static void send_start_line_and_headers(std::ostringstream &oss, FILE *output);
    bool is_file_in_directory(const std::string &file_path) const;
    static void send_response_with_file(const HttpRequest &request,
                                        FILE *output,
                                        const std::string &file_path_string);
    void handle_http_request(const HttpRequest &request, FILE *output) const;
    static void send_fail_response(int status_code, FILE *output);
    void communicate_with_client(int msg_sock);
    void set_communicaion_with_client();

  public:
    static Server create_from_program_arguments(int argc, char *argv[]);
    [[noreturn]] void run();
};

#endif //SIK1__SERVER_H_
