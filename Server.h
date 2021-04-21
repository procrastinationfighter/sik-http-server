#ifndef SIK1__SERVER_H_
#define SIK1__SERVER_H_

#include <netinet/in.h>
#include "server_utilities.h"
#include "HttpRequest.h"

class Server {
  private:
    std::string files_directory;
    std::string correlated_servers_file;

    int sock;
    struct sockaddr_in server_address;

    Server(std::string &&files_dir, std::string &&correlated_servers_file, int port);

    void communicate_with_client(int msg_sock);
    void set_communicaion_with_client();
    void handle_http_request(const HttpRequest &request) const;

  public:
    static Server create_from_program_arguments(int argc, char *argv[]);
    [[noreturn]] void run();
};

#endif //SIK1__SERVER_H_
