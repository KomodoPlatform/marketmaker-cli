
#ifndef MARKETMAKER_CLI_CPPSOCKET_H
#define MARKETMAKER_CLI_CPPSOCKET_H

#include <socket.h>
#include <string>

class CppSocket : AbstractSocket {
public:
    CppSocket();

    virtual bool doConnect(const URL *url, int tmout_ms, err_t *errp) = 0;

    virtual bool doWrite(const void *data, size_t len, err_t *errp) = 0;

    virtual char *doReadText(const std::string &terminator, int tmout_ms, err_t *errp) = 0;

    virtual void *doReadBinary(size_t expectedSize, size_t *actualSize, int tmout_ms, err_t *errp) = 0;

    virtual void doDisconnect() = 0;
};

#endif //MARKETMAKER_CLI_CPPSOCKET_H
