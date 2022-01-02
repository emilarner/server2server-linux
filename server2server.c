#include "server2server.h"

Server2Server *init_server(char *remote_addr, int remote_port, int control_port, int local_port,
                            enum Modes mode)
{
    Server2Server *srv = (Server2Server*) malloc(sizeof(*srv));
    srv->mode = mode;

    /* Detect if port outside of uint16_t range. */
    if ((uint32_t) remote_port > 65535 || (uint32_t) control_port > 65536 
            || (uint32_t) local_port > 65535)
    {
        fprintf(stderr, "Critical error: a port is above 65535!\n");
        return NULL; 
    }

    srv->cport = htons(control_port);
    srv->rport = htons(remote_port);
    srv->lport = htons(local_port);

    struct hostent *tmp = gethostbyname(remote_addr);

    /* The domain doesn't exist or the address is malformed. */
    if (tmp == NULL)
    {
        fprintf(stderr, "Error resolving address %s", remote_addr);
        herror("; this is the error");
        return NULL; 
    }

    /* This looks weird, but it is necessary to cast char** to struct in_addr* and then */
    /* dereference it. It is just a quirk about how gethostbyname() works. */
    /* gethostbyname() is deprecated, where getaddrinfo() is now preferred. */

    srv->raddr = *((struct in_addr*) tmp->h_addr_list[0]);
    return srv;
}


void start_server(Server2Server *srv)
{
    control(srv);
}

int make_connection(struct in_addr addr, uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in saddr;

    saddr.sin_family = AF_INET;
    saddr.sin_addr = addr;
    saddr.sin_port = port;

    /* Make a connection to the address/port. */
    if (connect(sockfd, (struct sockaddr*) &saddr, (socklen_t) sizeof(saddr)) < 0)
    {
        fprintf(stderr, "%s:%d", inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));
        perror(" failed on connect()");
        return -1;
    }

    return sockfd;
}

int make_udp_connection(in_addr_t eaddr, uint16_t port, struct UDPInfo *mut)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* DGRAMS (datagrams) are connectionless. */ 

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = eaddr;
    addr.sin_port = port;

    mut->addr = addr;
    mut->fd = fd;
    mut->len = sizeof(struct sockaddr_in);

    return fd; 
}

bool recvallsendall(int from, int to)
{
    char buffer[RECV_BUFFER_SIZE];
    ssize_t length = 0;

    /* If recv() returns -1 with MSG_DONTWAIT, errno is likely EWOULDBLOCK. */ 
    while ((length = recv(from, buffer, sizeof(buffer), MSG_DONTWAIT)) != -1)
    {
        /* If recv() returns 0, the socket has closed, so inform our handler function. */
        if (length == 0)
            return false; 

        send(to, buffer, length, 0);
    }

    return true;
}

bool udp_recvallsendall(struct UDPInfo *from, struct UDPInfo *to)
{
    char buffer[RECV_BUFFER_SIZE];
    ssize_t length = 0;

    /* Just the UDP way of doing things. Same principles as in the TCP version apply. */
    while ((length = recvfrom(from->fd, buffer, sizeof(buffer), MSG_DONTWAIT, 
                                (struct sockaddr*) &from->addr, &from->len)) != -1)
    {
        if (length == 0)
            return false; 

        sendto(to->fd, buffer, length, 0, (struct sockaddr*) &to->addr, to->len);
    }

    return true;
}

void *glue(void *srv)
{
    Server2Server *s = (Server2Server*) srv;

    /* Localhost. I tried INADDR_LOOPBACK, but that didn't seem to work... */
    /* Maybe 127.0.0.1 isn't INADDR_LOOPBACK? Or perhaps I need to convert it to big endian? */ 

    struct in_addr localaddr;
    localaddr.s_addr = inet_addr("127.0.0.1");

    /* Leave these here if UDP is our mode. */ 
    struct UDPInfo ulocal;
    struct UDPInfo uremote;

    int local = 0;
    int remote = 0; 

    /* Use the appropriate functions depending on the mode. */
    switch (s->mode)
    {
        case TCPMode:
        {    
            local = make_connection(localaddr, s->lport);
            remote = make_connection(s->raddr, s->rport);
            break;
        }

        case UDPMode:
        {
            local = make_udp_connection(localaddr.s_addr, s->lport, &ulocal); 
            remote = make_udp_connection(s->raddr.s_addr, s->rport, &uremote);
            break;
        }
    }

    /* Failure */
    if (local == -1 || remote == -1)
        return NULL; 

    struct timeval timeout;
    timeout.tv_sec = SELECT_TIMEOUT_SEC; 
    timeout.tv_usec = 0;

    fd_set fset;

    while (1)
    {
        FD_ZERO(&fset);

        FD_SET(local, &fset);
        FD_SET(remote, &fset);

        int ret = select(SELECT_FDS, &fset, NULL, NULL, &timeout);

        if (ret == -1)
        {
            perror("select() failed: ");
            return NULL;
        }
        else if (ret == 0)
        {
            fprintf(stderr, "select() timed out\n");
            return NULL;
        }
        else
        {
            if (FD_ISSET(local, &fset))
            {
                bool active = true;

                switch (s->mode)
                {
                    case TCPMode:
                    {
                        active = recvallsendall(local, remote);
                        break;
                    }

                    case UDPMode:
                    {
                        active = udp_recvallsendall(&ulocal, &uremote);
                        break;
                    }
                }

                if (!active)
                {
                    close(remote);
                    return NULL;
                }
            }

            if (FD_ISSET(remote, &fset))
            {
                bool active = true;

                switch (s->mode)
                {
                    case TCPMode:
                    {
                        active = recvallsendall(remote, local);
                        break;
                    }

                    case UDPMode:
                    {
                        active = udp_recvallsendall(&uremote, &ulocal);
                        break;
                    }
                }

                if (!active)
                {
                    close(remote);
                    return NULL;
                }
            }
        }

    }
}

void *control(Server2Server *s)
{
    while (true)
    {
        int fd = make_connection(s->raddr, s->cport);

        if (fd == -1)
        {
            perror("connnection to control server failed");
            control_again();
        }

        while (true)
        {
            char command[256];
            memset(command, 0, sizeof(command));

            ssize_t len = recv(fd, command, sizeof(command), 0);

            if (len <= 0)
            {
                perror("control connection died or suffered an error");
                control_again();
            }

            /* Someone has connected! */ 
            if (!memcmp(command, "CONNECT", sizeof("CONNECT") - 1))
            {
                printf("Received a connection request.\n");

                pthread_t tmp;
                pthread_create(&tmp, NULL, &glue, (void*) s); 
            }
        }
    }
}

void free_server(Server2Server *srv)
{
    free(srv);
}