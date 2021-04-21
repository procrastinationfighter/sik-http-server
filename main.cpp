#include "Server.h"

int main(int argc, char *argv[]) {
    Server server = Server::create_from_program_arguments(argc, argv);
    server.run();

    return 0;
}
