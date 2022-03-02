#include <winsock2.h>
#include <WS2tcpip.h>

static int find(const char* str, char c) {
    for (int i = 0; str[i]; ++i)
        if (str[i] == c)
            return i;
    return -1;
}

static unsigned int str_len(const char* str) {
    unsigned int i = 0;
    while (str[i]) ++i;
    return i;
}

static void mem_cpy(unsigned char* dest,
                    const unsigned char* src,
                    unsigned int count) {
    while (count--) *dest++ = *src++;
}

static void fwd_app_str(char** in_out_it,
                        const char* src) {
    while (*src) {
        **in_out_it = *src++;
        ++*in_out_it;
    }
}

static void utoa_dec(unsigned int num, char* buf) {
    int i = 0;

    if (num == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    while (num != 0) {
        int rem = num % 10;
        buf[i++] = rem > 9 ? rem - 10 + 'a' : rem + '0';
        num /= 10;
    }

    buf[i] = '\0';

    for (int start = 0, end = i - 1; start < end;) {
        i = (int)buf[start];
        buf[start++] = buf[end];
        buf[end--] = (char)i;
    }
}

enum sockerr_e {
    SOCKERR_OK,
    WSA_STARTUP_FAILED,
    GETHOSTBYNAME_FAILED,
    SOCKET_INIT_FAILED,
    CONNECT_FAILED,
    SEND_FAILED,
    SHUTDOWN_FAILED,
    RECV_FAILED,
    CLOSESOCKET_FAILED
};

enum sockerr_e PerformRequest(const char* url,
                              const char* method,
                              const char* data,
                              char* out_buf,
                              unsigned int out_buf_max_size) {
    // todo: add custom headers support
    int resbuf;
    WSADATA wsa_data;
    SOCKET consock;
    SOCKADDR_IN sockaddr;
    struct hostent* host;

    char hostname[128];
    char path[256];
    char content_len[16];
    const char* const chunk1 = " HTTP/1.1\r\nConnection: close\r\nHost: ";
    const char* const chunk2 = "\r\nContent-length: ";

// todo: char port[8];

    resbuf = find(url, '/');
    if (resbuf > -1) {
        mem_cpy(hostname, url, resbuf);
        hostname[resbuf] = '\0';

        const unsigned int buf = str_len(url) - resbuf;
        mem_cpy(path, url + resbuf, buf);
        path[buf] = '\0';
    } else {
        mem_cpy(hostname, url, str_len(url) + 1);
        path[0] = '/';
        path[1] = '\0';
    }
   
    resbuf = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (resbuf != 0) return WSA_STARTUP_FAILED;

    consock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (consock == INVALID_SOCKET) { WSACleanup(); return SOCKET_INIT_FAILED; }

    host = gethostbyname(hostname);
    if (host == NULL) { WSACleanup(); return GETHOSTBYNAME_FAILED; }
    
    sockaddr.sin_port = htons(80);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    resbuf = connect(consock, (SOCKADDR*)(&sockaddr), sizeof(sockaddr));
    if (resbuf != 0) { WSACleanup(); return CONNECT_FAILED; }

    resbuf = (int)str_len(data);
    utoa_dec(resbuf, content_len);

    const unsigned int req_str_len = str_len(chunk1)
                                   + str_len(chunk2)
                                   + str_len(content_len)
                                   + resbuf
                                   + str_len(hostname)
                                   + str_len(method)
                                   + str_len(path)
                                   + str_len(url)
                                   + 8; // " ", "\r\n\r\n", "\r\n\0"

    char* const req_str = HeapAlloc(GetProcessHeap(), 0ul, req_str_len);
    char* it = req_str;

    fwd_app_str(&it, method);
    fwd_app_str(&it, " ");
    fwd_app_str(&it, path);
    fwd_app_str(&it, chunk1);
    fwd_app_str(&it, hostname);
    fwd_app_str(&it, chunk2);
    fwd_app_str(&it, content_len);
    fwd_app_str(&it, "\r\n\r\n");
    fwd_app_str(&it, data);
    fwd_app_str(&it, "\r\n");
    *it = '\0';

    resbuf = send(consock, req_str, (int)req_str_len, 0);
    if (resbuf < 0) { WSACleanup(); return SEND_FAILED; }

    resbuf = recv(consock, out_buf, out_buf_max_size - 1, 0);
    if (resbuf < 0) { WSACleanup(); return RECV_FAILED; }
    out_buf[resbuf] = '\0';

    resbuf = closesocket(consock); // todo: check return
    WSACleanup();
    if (resbuf != 0) return CLOSESOCKET_FAILED;
    
    return SOCKERR_OK;
}
