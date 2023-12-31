#include "openssl/ssl.h"
#include "openssl/err.h"

#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#define SOCKET_ERROR -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define PORT 443
#define READ_BUFFER_SIZE 1000000

void log_ssl() {
    int err = ERR_get_error();
    const char* str = ERR_lib_error_string(err);
    const char* reason = ERR_reason_error_string(err);

    printf("LOG SSL: code=%d, human='%s', reason='%s'\n", err, str, reason);
}

int send_request(SSL* ssl, const char* request) {
    int len = SSL_write(ssl, request, strlen(request));
    log_ssl();
    if (len < 0) {
        int err = SSL_get_error(ssl, len);
        printf("Error sending request. Error=%d\n", err);
        return 0;
    }
    return len;
}

int read_response(SSL* ssl) {
    int buf[READ_BUFFER_SIZE];
    int len = SSL_read(ssl, buf, READ_BUFFER_SIZE);
    log_ssl();
    if (len < 0) {
        int err = SSL_get_error(ssl, len);
        printf("Error sending request. Error=%d\n", err);
        return 0;
    }
    printf("Reponse:\n%s", buf);
    return len;
}

int create_connection_to(const char* hostname) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Cannot create socket");
        exit(1);
    }
    struct hostent* hostinfo = NULL;
    SOCKADDR_IN sin = { 0 };
    hostinfo = gethostbyname(hostname);
    if (hostinfo == NULL) {
        printf ("Unknown host %s.\n", hostname);
        exit(1);
    }

    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_port = htons(PORT);
    sin.sin_family = AF_INET;

    if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        perror("connect()");
        exit(errno);
    }
    return sock;
}

int main(int argc, char *argv[])
{
    const char* hostname = "stac47.github.io";
    printf("Accessing %s through TLS\n", hostname);

    int sock = create_connection_to(hostname);

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    SSL_CTX* ctx = SSL_CTX_new(TLS_method());
    log_ssl();
    printf("SSL CTX created at %p\n", ctx);
    SSL* ssl = SSL_new(ctx);
    log_ssl();
    printf("SSL created at %p\n", ssl);
    if (SSL_set_fd(ssl, sock)) {
        printf("FD %d assigned to SSL at %p\n", sock, ssl);
    }
    else {
        puts("An error occured");
        log_ssl();
        exit(1);
    }

    int err = SSL_connect(ssl);
    if (err == 1) {
        puts("TLS handshake success");
    } else {
        log_ssl();
        printf("Error sending request. Error=%d\n", err);
        exit(1);
    }
    const char* request = "GET https://stac47.github.io/ HTTP/1.1\r\n\r\n";
    send_request(ssl, request);
    read_response(ssl);

    close(sock);
    /* free(hostinfo); */
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return 0;
}
