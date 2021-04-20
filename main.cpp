#include "Server.h"

int main(int argc, char *argv[]) {
    // [TODO] 1. Check arguments. (first: directory, second: file, third (optional): port)
    //        2. Start server and wait for clients.
    //        3. Parse queries.
    Server server = Server::create_from_program_arguments(argc, argv);
    server.run();

    return 0;
}
