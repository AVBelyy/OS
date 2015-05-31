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

int create_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
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
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    memcpy(&addr, result->ai_addr, result->ai_addrlen); // use first address available
    freeaddrinfo(result);
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        return -1;
    }
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return -1;
    }
    return sockfd;
}

int create_client(int sockfd, struct sockaddr_in * client) {
    int clientfd = accept(sockfd, (struct sockaddr *) client, &(socklen_t){sizeof(struct sockaddr_in)});
    if (clientfd == -1) {
        if (errno != EINTR) {
            perror("accept");
        }
        return -1;
    }
    return clientfd;
}

void start_client(int fromfd, int tofd) {
    printf("client %d connected\n", fromfd);

    struct buf_t * buf = buf_new(4096);
    ssize_t nread, nwritten;

    do {
        nread = buf_fill(fromfd, buf, buf_capacity(buf));
        nwritten = buf_flush(tofd, buf, buf_size(buf));
        if (nread == -1 || nwritten == -1) {
            perror("client::r/w");
            break;
        }
    } while (nread > 0);

    printf("client %d disconnected\n", fromfd);
    exit(0);
}

int main(int argc, char * argv[]) {
    if (argc != 3) {
        printf("usage: %s port1 port2\n", argv[0]);
        return 1;
    }

    // Parse args
    int port1, port2;
    sscanf(argv[1], "%d", &port1);
    sscanf(argv[2], "%d", &port2);

    // Initialize servers
    int sockfd1 = create_server(port1);
    if (sockfd1 == -1) {
        return 1;
    }

    int sockfd2 = create_server(port2);
    if (sockfd2 == -1) {
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
    printf("listening on ports %d and %d\n", port1, port2);

    // Accept pairs of clients
    struct sockaddr_in client1, client2;
    while (1) {
        int clientfd1 = create_client(sockfd1, &client1);
        if (clientfd1 == -1) {
            if (errno == EINTR) {
                break;
            } else {
                return 1;
            }
        }
        int clientfd2 = create_client(sockfd2, &client2);
        if (clientfd2 == -1) {
            if (errno == EINTR) {
                break;
            } else {
                return 1;
            }
        }
        if (fork() == 0) {
            // Client 1
            start_client(clientfd1, clientfd2);
        }
        if (fork() == 0) {
            // Client 2
            start_client(clientfd2, clientfd1);
        }
        close(clientfd1);
        close(clientfd2);
    }

    printf("fare thee well\n");
    return 0;
}
