#ifndef SERVER2SERVER_H
#define SERVER2SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <pthread.h>
#include <sys/select.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "config.h"

#define MILLISECOND 1000
#define SECOND 1000
#define CONTROL_SLEEP_MS 10 * SECOND

#define control_again() do {                                \
usleep(CONTROL_SLEEP_MS * MILLISECOND);                     \
continue; } while (0);                                      \


enum Modes
{
    UndefinedMode,
    TCPMode,
    UDPMode
};

struct UDPInfo
{
    struct sockaddr_in addr;
    socklen_t len;
    int fd;
};

struct Server2Server
{
    enum Modes mode; 
    struct in_addr raddr;

    uint16_t rport;
    uint16_t cport;
    uint16_t lport;

    pthread_t control;
};

typedef struct Server2Server Server2Server;

Server2Server *init_server(char *remote_addr, int remote_port, int control_port, int local_port,
                            enum Modes mode);

void start_server(Server2Server *srv);

int make_connection(struct in_addr addr, uint16_t port);
int make_udp_connection(in_addr_t eaddr, uint16_t port, struct UDPInfo *mut);

void *control(Server2Server *s);
void *glue(void *srv);

bool recvallsendall(int from, int to); 
bool udp_recvallsendall(struct UDPInfo *from, struct UDPInfo *to);

void free_server(Server2Server *srv);





#endif