/// @file testSockhost.cc Test sockets server host

#include "ConfigFactory.hh"
#include "SockBinIO.hh"
#include "GlobalArgs.hh"

REGISTER_EXECLET(Sockhost) {
    SockConnection SC("localhost", 50000);
    optionalGlobalArg("host", SC.host, "socket server hostname");
    optionalGlobalArg("port", SC.port, "socket server port");

    SC.create_socket();
    printf("Opened socket at file descriptor %i\n", SC.sockfd);
    SockBinRead SC2(SC.awaitConnection());
    printf("Received connection at file descriptor %i\n", SC2.sockfd);

    string s;
    while(1) {
        SC2.receive(s);
        if(!s.size()) break;
        printf("\t'%s'\n", s.c_str());
    }
}
