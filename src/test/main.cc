#include <gtest/gtest.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

int main(int argc, char **argv)
{
#ifdef _WIN32
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed: %d\n", rc);
        return 1;
    }
#endif
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}