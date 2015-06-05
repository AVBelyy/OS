#define _POSIX_SOURCE

#include <bufio.h>
#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CLIENTS 127
#define BUF_SIZE 4096

struct pollfd polls[2 * CLIENTS + 2];
struct buf_t * buffs[2 * CLIENTS];
size_t clients;

void do_nothing(int signo) {
}

int create_server(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
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

int create_client_and_poll_existing(int sockfd) {
    int polls_count = 2 * clients + 2;
    static char buf[BUF_SIZE];

    for (size_t i = 0; i < 2; i++) {
        if (polls[i].fd == sockfd && clients < CLIENTS) {
            polls[i].events = POLLIN;
        } else {
            polls[i].events = 0;
        }
    }

    while (1) {
        // set reading / writing permissions
        for (size_t i = 0; i < clients; i++) {
            size_t buf1size = buffs[2 * i]->size;
            size_t buf2size = buffs[2 * i + 1]->size;
            polls[2 * i + 2].events  = buf1size == BUF_SIZE ? 0 : POLLIN;
            polls[2 * i + 2].events |= buf2size == 0 ? 0 : POLLOUT;
            polls[2 * i + 3].events  = buf1size == 0 ? 0 : POLLOUT;
            polls[2 * i + 3].events |= buf2size == BUF_SIZE ? 0 : POLLIN;
        }
        // poll loop
        int res = poll(polls, polls_count, -1);
        if (res < 0) {
            return -1;
        }
        for (size_t i = 0; i < polls_count; i++) {
            size_t j = i ^ 1;
            if (polls[i].revents & POLLIN) {
                if (polls[i].fd == sockfd) {
                    return accept(sockfd, NULL, NULL);
                } else {
                    size_t cap = BUF_SIZE - buffs[i - 2]->size;
                    ssize_t nread = recv(polls[i].fd, &buf, cap, 0);
                    if (nread > 0) {
                        buf_put(buffs[i - 2], buf, nread);
                    }
                }
            }
            if (polls[i].revents & POLLOUT) {
                size_t cap = buffs[j - 2]->size;
                ssize_t nwritten = send(polls[i].fd, buffs[j - 2]->data, cap, 0);
                if (nwritten > 0) {
                    buffs[j - 2]->size = 0;
                }
            }
            if (polls[i].revents & POLLHUP) {
                return -(i & ~1);
            }
        }
    }
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
    signal(SIGPIPE, SIG_IGN);

    // Greeting message
    printf("listening on ports %d and %d\n", port1, port2);

    // Initialize polls
    polls[0].fd = sockfd1;
    polls[1].fd = sockfd2;

    // Accept clients, transmit data, close clients -- all in one single loop!
    while (1) {
        int clientfd1 = create_client_and_poll_existing(sockfd1);
        if (clientfd1 == -1) {
            if (errno == EINTR) {
                break;
            } else {
                return 1;
            }
        }
        
        int clientfd2 = 0;
        if (clientfd1 > 0) {
            clientfd2 = create_client_and_poll_existing(sockfd2);
            if (clientfd2 == -1) {
                if (errno == EINTR) {
                    break;
                } else {
                    return 1;
                }
            }
        }
        
        if (clientfd1 < 0 || clientfd2 < 0) {
            // we have lost a pair of clients
            size_t k = clientfd1 < 0 ? -clientfd1 : -clientfd2;
            printf("clients %d, %d disconnected\n", polls[k].fd, polls[k + 1].fd);
            close(polls[k].fd);
            close(polls[k + 1].fd);
            buffs[k - 2] = buffs[2 * clients - 2];
            buffs[k - 1] = buffs[2 * clients - 1];
            polls[k].fd = polls[2 * clients].fd;
            polls[k + 1].fd = polls[2 * clients + 1].fd;
            clients--;
        } else {
            // we have a new pair of clients
            printf("new clients %d, %d\n", clientfd1, clientfd2);
            buffs[2 * clients] = buf_new(BUF_SIZE);
            buffs[2 * clients + 1] = buf_new(BUF_SIZE);
            polls[2 * clients + 2].fd = clientfd1;
            polls[2 * clients + 3].fd = clientfd2;
            clients++;
        }
    }

    printf("fare thee well\n");
    return 0;
}
