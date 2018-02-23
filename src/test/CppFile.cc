
#include "CppFile.h"

static bool _open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp);

static long _size(AbstractFile *absFile, err_t *errp);

static size_t _read(AbstractFile *absFile, void *ptr, size_t size, err_t *errp);

static bool _write(AbstractFile *absFile, const void *ptr, size_t size, err_t *errp);

static void _close(AbstractFile *absFile);

CppFile::CppFile()
{
    this->open = _open;
    this->size = _size;
    this->read = _read;
    this->write = _write;
    this->close = _close;
}

bool _open(AbstractFile *absFile, const char *pathname, const char *mode, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doOpen(pathname, mode, errp);
}

long _size(AbstractFile *absFile, err_t *errp)
{
    auto *file = (CppFile *) absFile;
    return file->doSize(errp);
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
