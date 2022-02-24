#include <winsock2.h>
#include <string.h>
#include <string>

static int find(const char* inStr, char c) {
    for (int i = 0; *(inStr + i); ++i)
        if (inStr[i] == c)
            return i;
    return -1;
}

const char* PerformRequest(const char* url,
                           const char* method,
                           const char* data,
                           char* outBuf,
                           int outBufMaxSize) {
    static const char* c = "xdd";

    char path[256];
    char host[128];
    char domain[128];
    char port[8];

    const int pathStartPos = find(url, '/');

    if (pathStartPos == -1) {
        std::string s;
    }

    

    


    



    return outBuf;
}