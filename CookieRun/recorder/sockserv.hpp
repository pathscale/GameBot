namespace sserv {
class Socket {
public:
    int fd;
    int write_(const void *data, int data_size);
    inline int error() {
        return fd <= 0;
    }
};
int close_(Socket s);


class SocketServer {
public:
    int fd;
    inline int error() {
        return fd <= 0;
    }
};
SocketServer get_bound_sock(int port);
Socket accept(SocketServer server);
int close_(SocketServer ss);

}
