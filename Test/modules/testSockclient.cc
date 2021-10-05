/// \file testSockclient.cc Test sockets server client

#include "ConfigFactory.hh"
#include "SockBinIO.hh"
#include "GlobalArgs.hh"

REGISTER_EXECLET(Sockclient) {
    SockBinWrite SC("localhost", 50000);
    optionalGlobalArg("host", SC.host, "socket server hostname");
    optionalGlobalArg("port", SC.port, "socket server port");

    SC.connect_to_socket();
    printf("Opened socket at file descriptor %i\n", SC.sockfd);

    usleep(1000000);

    SC.send("hello");
    SC.send("world");

    usleep(1000000);

    SC.send("Mary");
    SC.send("had");
    SC.send("a");
    SC.send("little");
    SC.send("lamb");
    SC.send("");
}
