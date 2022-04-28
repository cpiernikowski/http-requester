#include <winsock2.h>
#include <WS2tcpip.h>

static SIZE_T Find(PCSTR str, CHAR c) {
    for (SIZE_T i = 0ull; str[i] != '\0'; ++i)
        if (str[i] == c)
            return i;
    return (SIZE_T)-1;
}

static SIZE_T StrLen(PCSTR str) {
    SIZE_T i = 0ull;
    while (str[i] != '\0') ++i;
    return i;
}

static VOID MemCpy(BYTE* dest,
                   CONST BYTE* src,
                   SIZE_T count) {
    while (count-- > 0) *dest++ = *src++;
}

static VOID FwdAppStr(PZPSTR in_out_it, PCSTR src) {
    while (*src) {
        **in_out_it = *src++;
        ++*in_out_it;
    }
}

static VOID UtoaDec(SIZE_T num, PSTR buf) {
    SIZE_T i = 0ull;

    if (num == 0ull) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    while (num != 0ull) {
        buf[i++] = '0' + (num % 10ull);
        num /= 10ull;
    }

    buf[i] = '\0';

    for (SIZE_T start = 0ull, end = i - 1ull; start < end;) {
        i = (SIZE_T)buf[start];
        buf[start++] = buf[end];
        buf[end--] = (CHAR)i;
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

static inline enum sockerr_e SockCleanup(SOCKET sock,
                                         enum sockerr_e resval) {
    closesocket(sock);
    WSACleanup();
    return resval;
}

enum sockerr_e PerformRequest(PCSTR url,
                              PCSTR method,
                              PCSTR data,
                              PSTR out_buf,
                              UINT out_buf_max_size) {
    INT resbuf;
    WSADATA wsa_data;
    SOCKET consock;
    SOCKADDR_IN sockaddr;
    struct hostent* host;

    CHAR hostname[128];
    CHAR path[256];
    CHAR content_len[16];
    CONST PCSTR chunk1 = " HTTP/1.1\r\nConnection: close\r\nHost: ";
    CONST PCSTR chunk2 = "\r\nContent-length: ";

    CONST SIZE_T path_start_pos = Find(url, '/');
    if (path_start_pos > 128ull && path_start_pos != (SIZE_T)-1)
        return OUT_OF_MEM;

    if (path_start_pos != (SIZE_T)-1) {
        MemCpy(hostname, url, path_start_pos);
        hostname[path_start_pos] = '\0';

        CONST SIZE_T pathlen = StrLen(url) - path_start_pos;
        if (pathlen > 255ull)
            return OUT_OF_MEM;
        MemCpy(path, url + path_start_pos, pathlen);
        path[pathlen] = '\0';
    } else {
        CONST SIZE_T urllen = StrLen(url);
        if (urllen > 127ull)
            return OUT_OF_MEM;
        MemCpy(hostname, url, urllen + 1);
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
        return SockCleanup(consock, GETHOSTBYNAME_FAILED);
    
    sockaddr.sin_port = htons((USHORT)80);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = *((ULONG*)host->h_addr);

    resbuf = connect(consock, (SOCKADDR*)(&sockaddr), sizeof(sockaddr));
    if (resbuf != 0)
        return SockCleanup(consock, CONNECT_FAILED);

    CONST SIZE_T data_str_len = StrLen(data);
    UtoaDec(data_str_len, content_len);

    CONST SIZE_T req_str_len = data_str_len
                             + StrLen(chunk1)
                             + StrLen(chunk2)
                             + StrLen(content_len)
                             + StrLen(hostname)
                             + StrLen(method)
                             + StrLen(path)
                             + StrLen(url)
                             + 8; // " ", "\r\n\r\n", "\r\n\0"

    HANDLE heap = GetProcessHeap();
    CONST PSTR req_str = HeapAlloc(heap, 0ul, req_str_len * sizeof(char));
    if (req_str == NULL)
        return SockCleanup(consock, OUT_OF_MEM);

    PSTR it = req_str;
    FwdAppStr(&it, method);
    FwdAppStr(&it, " ");
    FwdAppStr(&it, path);
    FwdAppStr(&it, chunk1);
    FwdAppStr(&it, hostname);
    FwdAppStr(&it, chunk2);
    FwdAppStr(&it, content_len);
    FwdAppStr(&it, "\r\n\r\n");
    FwdAppStr(&it, data);
    FwdAppStr(&it, "\r\n");
    *it = '\0';

    resbuf = send(consock, req_str, (INT)req_str_len, 0);
    HeapFree(heap, 0ul, req_str);
    if (resbuf < 0)
        return SockCleanup(consock, SEND_FAILED);

    resbuf = recv(consock, out_buf, out_buf_max_size - 1, 0);
    if (resbuf < 0)
        return SockCleanup(consock, RECV_FAILED);
    out_buf[resbuf] = '\0';

    resbuf = closesocket(consock);
    WSACleanup();
    if (resbuf != 0)
        return CLOSESOCKET_FAILED;

    return SOCKERR_OK;
}
