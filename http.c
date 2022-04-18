#include <winsock2.h>
#include <WS2tcpip.h>

typedef unsigned long long u64;

static u64 find(const char* str, char c) {
    for (int i = 0; str[i] != '\0'; ++i)
        if (str[i] == c)
            return i;
    return (u64)-1;
}

static u64 str_len(const char* str) {
    u64 i = 0;
    while (str[i] != '\0') ++i;
    return i;
}

static void mem_cpy(unsigned char* dest,
                    const unsigned char* src,
                    u64 count) {
    while (count-- > 0) *dest++ = *src++;
}

static void fwd_app_str(char** in_out_it,
                        const char* src) {
    while (*src) {
        **in_out_it = *src++;
        ++*in_out_it;
    }
}

static void utoa_dec(u64 num, char* buf) {
    int i = 0;

    if (num == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    while (num != 0) {
        buf[i++] = '0' + (num % 10);
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
    RECV_FAILED,
    CLOSESOCKET_FAILED,
    OUT_OF_MEM
};

static inline enum sockerr_e sock_cleanup(SOCKET sock,
                                          enum sockerr_e resval) {
    closesocket(sock);
    WSACleanup();
    return resval;
}

enum sockerr_e PerformRequest(const char* url,
                              const char* method,
                              const char* data,
                              char* out_buf,
                              unsigned int out_buf_max_size) {
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

    const u64 path_start_pos = find(url, '/');
    if (path_start_pos > 128)
        return OUT_OF_MEM;

    if (path_start_pos != (u64)-1) {
        mem_cpy(hostname, url, path_start_pos);
        hostname[path_start_pos] = '\0';

        const u64 pathlen = str_len(url) - path_start_pos;
        if (pathlen > 255)
            return OUT_OF_MEM;
        mem_cpy(path, url + path_start_pos, pathlen);
        path[pathlen] = '\0';
    } else {
        const u64 urllen = str_len(url);
        if (urllen > 127)
            return OUT_OF_MEM;
        mem_cpy(hostname, url, urllen + 1);
        path[0] = '/';
        path[1] = '\0';
    }
   
    resbuf = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (resbuf != 0)
        return WSA_STARTUP_FAILED;

    consock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (consock == INVALID_SOCKET) {
        // no need to call closesocket()
        WSACleanup();
        return SOCKET_INIT_FAILED;
    }

    host = gethostbyname(hostname);
    if (host == NULL)
        return sock_cleanup(consock, GETHOSTBYNAME_FAILED);
    
    sockaddr.sin_port = htons(80);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    resbuf = connect(consock, (SOCKADDR*)(&sockaddr), sizeof(sockaddr));
    if (resbuf != 0)
        return sock_cleanup(consock, CONNECT_FAILED);

    const u64 data_str_len = str_len(data);
    utoa_dec(data_str_len, content_len);

    const u64 req_str_len = data_str_len
                          + str_len(chunk1)
                          + str_len(chunk2)
                          + str_len(content_len)
                          + str_len(hostname)
                          + str_len(method)
                          + str_len(path)
                          + str_len(url)
                          + 8; // " ", "\r\n\r\n", "\r\n\0"

    HANDLE heap = GetProcessHeap();
    char* const req_str = HeapAlloc(heap, 0ul, req_str_len * sizeof(char));
    if (req_str == NULL)
        return sock_cleanup(consock, OUT_OF_MEM);

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
    HeapFree(heap, 0ul, req_str);
    if (resbuf < 0)
        return sock_cleanup(consock, SEND_FAILED);

    resbuf = recv(consock, out_buf, out_buf_max_size - 1, 0);
    if (resbuf < 0)
        return sock_cleanup(consock, RECV_FAILED);
    out_buf[resbuf] = '\0';

    resbuf = closesocket(consock);
    WSACleanup();
    if (resbuf != 0)
        return CLOSESOCKET_FAILED;

    return SOCKERR_OK;
}
