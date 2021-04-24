#ifndef SIK1__SERVER_H_
#define SIK1__SERVER_H_

#include <netinet/in.h>
#include <filesystem>
#include "server_utilities.h"
#include "HttpRequest.h"

using correlated_server = std::pair<std::string, int>;

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
    std::map<std::string, correlated_server> correlated_server_files;

    SocketWrapper sock;
    struct sockaddr_in server_address;

    Server(std::string files_dir, const std::string& correlated_servers_file, int port);

    static void send_start_line_and_headers(std::ostringstream &oss, FILE *output);
    bool is_file_in_directory(const std::string &file_path) const;
    static void send_response_with_file(const HttpRequest &request,
                                        FILE *output,
                                        const std::string &file_path_string);
    void check_correlated_files(const HttpRequest &request, FILE *output) const;
    void handle_http_request(const HttpRequest &request, FILE *output) const;
    static void send_fail_response(int status_code, FILE *output);
    bool read_and_handle_request(FILE *input_file, FILE* output_file) const;
    void communicate_with_client(int msg_sock) const;
    void set_communication_with_client() const;

  public:
    static Server create_from_program_arguments(int argc, char *argv[]);
    [[noreturn]] void run() const;
};

#endif //SIK1__SERVER_H_
