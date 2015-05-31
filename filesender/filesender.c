#define _POSIX_SOURCE

#include <bufio.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void do_nothing(int signo) {
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("usage: %s port filepath\n", argv[0]);
        return 1;
    }

    // Parse args
    int port;
    sscanf(argv[1], "%d", &port);
    const char * filepath = argv[2];

    // Initialize server
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
    }
    struct addrinfo * result;
    struct addrinfo hints = {
        AI_V4MAPPED | AI_ADDRCONFIG, // ai_flags
        AF_INET,                     // ai_family
        SOCK_STREAM,                 // ai_socktype
        0, 0, 0, 0, 0                // not needed
    };
    hints.ai_next = NULL;
    int error = getaddrinfo("localhost", NULL, &hints, &result);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    memcpy(&addr, result->ai_addr, result->ai_addrlen); // use first address available
    freeaddrinfo(result);
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return 1;
    }

    // Set interrupt handler
    struct sigaction action = {
        .sa_handler = do_nothing,
        .sa_flags = 0
    };
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);

    // Greeting message
    printf("listening on port %d\n", port);

    // Accept clients
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    while (1) {
        int clientfd = accept(sockfd, (struct sockaddr *) &client, &sz);
        if (clientfd == -1) {
            if (errno == EINTR) {
                break;
            } else {
                perror("accept");
                return 1;
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            printf("client %d connected\n", clientfd);

            int filefd = open(filepath, O_RDONLY);
            if (filefd == -1) {
                perror("client::open");
                exit(0);
            }

            struct buf_t * buf = buf_new(4096);
            ssize_t nread, nwritten;

            do {
                nread = buf_fill(filefd, buf, buf_capacity(buf));
                nwritten = buf_flush(clientfd, buf, buf_size(buf));
                if (nread == -1 || nwritten == -1) {
                    perror("client::r/w");
                    break;
                }
            } while (nread > 0);

            printf("client %d disconnected\n", clientfd);
            exit(0);
        }
        close(clientfd);
    }

    printf("fare thee well\n");
    return 0;
}
