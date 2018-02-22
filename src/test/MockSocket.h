
#ifndef MARKETMAKER_CLI_MOCKSOCKET_H
#define MARKETMAKER_CLI_MOCKSOCKET_H

#include <gmock/gmock.h>

#include "CppSocket.h"

class MockSocket : public CppSocket {
public:
    MOCK_METHOD3(doConnect, bool(const URL *url, int tmout_ms, err_t*errp));
    MOCK_METHOD3(doWrite, bool(const void *data, size_t len, err_t *errp));
    MOCK_METHOD3(doReadText, char *(const char *terminator, int tmout_ms, err_t *errp));
    MOCK_METHOD4(doReadBinary, void *(size_t expectedSize, size_t *actualSize, int tmout_ms, err_t *err));
    MOCK_METHOD0(doDisconnect, void());
};

#endif //MARKETMAKER_CLI_MOCKSOCKET_H
