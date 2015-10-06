#include "sockserv.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

sserv::SocketServer sserv::get_bound_sock(int port) {
    int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == 0) {
        return {0};
    }
    int res;
    struct sockaddr_in bind_addr = {AF_INET, htons(port), INADDR_ANY};
    res = bind(listenfd, (struct sockaddr *)&bind_addr, sizeof(struct sockaddr_in));
    if (res) {
        std::cerr << "bind error " << res << std::endl;
        return {0};
    }
    res = listen(listenfd, 2);
    if (res) {
        return {0};
    }
    return {listenfd};
}

sserv::Socket sserv::accept(SocketServer server) {
    struct sockaddr_in peer_addr;
    socklen_t addrsize = sizeof(struct sockaddr_in);
    return {accept(server.fd, (struct sockaddr *)&peer_addr, &addrsize)};
}

int sserv::Socket::write_(const void *data, int data_size) {
    return write(this->fd, data, data_size);
}

int sserv::close_(sserv::SocketServer ss) {
    return close(ss.fd);
}

int sserv::close_(sserv::Socket s) {
    return close(s.fd);
}
