
#ifndef MARKETMAKER_CLI_CPPFILE_H
#define MARKETMAKER_CLI_CPPFILE_H

#include "file.h"

#include <string>

class CppFile : public AbstractFile {
public:
    CppFile();

    virtual bool doOpen(const std::string &pathname, const std::string &mode, err_t *errp) = 0;

    virtual long doSize(err_t *errp) = 0;

    virtual size_t doRead(char *ptr, size_t size, err_t *errp) = 0;

    virtual bool doWrite(const void *ptr, size_t size, err_t *errp) = 0;

    virtual void doClose() = 0;
};

#endif //MARKETMAKER_CLI_CPPFILE_H
