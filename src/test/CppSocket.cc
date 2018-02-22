#include "CppSocket.h"

static bool _connect(AbstractSocket *absSocket, const URL *url, int tmout_ms, err_t *errp);

static bool _write(AbstractSocket *absSocket, const void *data, size_t len, err_t *errp);

static char *_read_text(AbstractSocket *absSocket, const char *terminator, int tmout_ms, err_t *errp);

static void *_read_binary(AbstractSocket *absSocket, size_t expectedSize, size_t *actualSize, int tmout_ms,
                          err_t *errp);

static void _disconnect(AbstractSocket *absSocket);

CppSocket::CppSocket()
{
    this->connect = _connect;
    this->write = _write;
    this->read_text = _read_text;
    this->read_binary = _read_binary;
    this->disconnect = _disconnect;
}

bool _connect(AbstractSocket *absSocket, const URL *url, int tmout_ms, err_t *errp)
{
    auto *mock = (CppSocket *) absSocket;
    return mock->doConnect(url, tmout_ms, errp);
}

bool _write(AbstractSocket *absSocket, const void *data, size_t len, err_t *errp)
{
    auto *mock = (CppSocket *) absSocket;
    return mock->doWrite(data, len, errp);
}

char *_read_text(AbstractSocket *absSocket, const char *terminator, int tmout_ms, err_t *errp)
{
    auto *mock = (CppSocket *) absSocket;
    return mock->doReadText(terminator, tmout_ms, errp);
}

void *_read_binary(AbstractSocket *absSocket, size_t expectedSize, size_t *actualSize, int tmout_ms, err_t *errp)
{
    auto *mock = (CppSocket *) absSocket;
    return mock->doReadBinary(expectedSize, actualSize, tmout_ms, errp);
}

void _disconnect(AbstractSocket *absSocket)
{
    auto *mock = (CppSocket *) absSocket;
    mock->doDisconnect();
}
