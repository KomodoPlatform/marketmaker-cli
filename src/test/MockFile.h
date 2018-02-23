#ifndef MARKETMAKER_CLI_MOCKFILE_H
#define MARKETMAKER_CLI_MOCKFILE_H

#include "CppFile.h"

class MockFile : public CppFile {
public:
    MOCK_METHOD3(doOpen, bool(const std::string &pathname, const std::string &mode, err_t *errp));
    MOCK_METHOD1(doSize, long(err_t *errp));
    MOCK_METHOD3(doRead, size_t(char *ptr, size_t size, err_t *errp));
    MOCK_METHOD3(doWrite, bool(const void *ptr, size_t size, err_t *errp));
    MOCK_METHOD0(doClose, void());
};

#endif //MARKETMAKER_CLI_MOCKFILE_H
