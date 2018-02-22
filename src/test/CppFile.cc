
#include "CppFile.h"

static bool _open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp);

static bool _seek(AbstractFile *absFile, long offset, int whence, err_t *errp);

static long _tell(AbstractFile *absFile, err_t *errp);

static size_t _read(AbstractFile *absFile, void *ptr, size_t size, err_t *errp);

static bool _write(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp);

static void _close(AbstractFile *absFile);

CppFile::CppFile()
{
    this->open = _open;
    this->seek = _seek;
    this->tell = _tell;
    this->read = _read;
    this->write = _write;
    this->close = _close;
}

bool _open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doOpen(pathname, mode, errp);
}

bool _seek(AbstractFile *absFile, long offset, int whence, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doSeek(offset, whence, errp);
}

long _tell(AbstractFile *absFile, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doTell(errp);
}

size_t _read(AbstractFile *absFile, void *ptr, size_t size, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doRead((char *) ptr, size, errp);
}

bool _write(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doWrite(ptr, size, errp);
}

void _close(AbstractFile *absFile)
{
    auto *file = (CppFile *) absFile;
    file->doClose();
}
