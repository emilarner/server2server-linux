#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


#include "server2server.h"

#define arg_complain(name, next) if (next == NULL)                                      \
                    {                                                                   \
                        fprintf(stderr, "Error: " name " requires an argument.\n");     \
                        return -1;                                                      \
                    }                                                                   \

void help()
{
    const char *msg = 
    "server2server - Glues two servers together over the firewall, for firehop.\n"
    "Running the Linux/POSIX version.\n"
    "USAGE:\n"
    "server2server [args...]\n\n"
    "--remote, -r       [Required] Specifies the remote port to connect to.\n"
    "--local, -l        [Required] Specifies the local port to connect to.\n"
    "--control, -c      [Required] Specifies the control port to connect to.\n"
    "--address, -a      [Required] Specifies the address of the remote and control endpoints.\n"
    "--tcp, -t          [Default] Explicitly specifies TCP mode.\n"
    "--udp, -u          Specifies that the program should reverse a UDP service.\n"
    "--quiet, -q        Print absolutely nothing to the console.\n"
    "--help, -h         Print this menu, ceasing program operation afterwards.\n";

    printf("%s\n", msg);
}

int main(int argc, char **argv, char **envp)
{
    /* Initialization of variables to default values. */
    enum Modes mode = UndefinedMode;
    char *remoteaddr = NULL;
    int remotep = 0;
    int controlp = 0;
    int localp = 0; 

    /* Cheap way to argument parse; it works, anyways. */
    for (size_t i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "--local") || !strcmp(argv[i], "-l"))
            {
                arg_complain("--local/-l", argv[i + 1]);
                localp = atoi(argv[i + 1]);
            }

            else if (!strcmp(argv[i], "--remote") || !strcmp(argv[i], "-r"))
            {
                arg_complain("--remote/-r", argv[i + 1]);
                remotep = atoi(argv[i + 1]);
            }

            else if (!strcmp(argv[i], "--address") || !strcmp(argv[i], "-a"))
            {
                arg_complain("--address/-a", argv[i + 1]);
                remoteaddr = argv[i + 1];
            }

            else if (!strcmp(argv[i], "--control") || !strcmp(argv[i], "-c"))
            {
                arg_complain("--control/-c", argv[i + 1]);
                controlp = atoi(argv[i + 1]);
            }

            else if (!strcmp(argv[i], "--udp") || !strcmp(argv[i], "-u"))
                mode = UDPMode;

            else if (!strcmp(argv[i], "--tcp") || !strcmp(argv[i], "-t"))
                mode = TCPMode;

            else if (!strcmp(argv[i], "--quiet") || !strcmp(argv[i], "-q"))
            {
                FILE *fp = fopen("/dev/null", "wb");
                dup2(fileno(fp), STDOUT_FILENO);
                dup2(fileno(fp), STDERR_FILENO);
            }
            else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
            {
                help();
                return 0;
            }
            else
            {
                fprintf(stderr, "Error: you have provided an invalid argument.\n");
                help();
                return -2; 
            }
        }
    }

    /* Sanity checking. */
    if (mode == UndefinedMode)
    {
        fprintf(stderr, "Warning: no mode explicitly supplied, so I'm assuming TCP.\n");
        mode = TCPMode; 
    }

    if (!remotep || !localp || !controlp)
    {
        fprintf(stderr, "Error: you are missing a required port. Check your arguments?\n");
        help();
        return -3;
    }

    /* By default, its localhost. */
    if (remoteaddr == NULL)
    {
        fprintf(stderr, "Warning: you did not supply an address, so I'm assuming 127.0.0.1.\n");
        remoteaddr = "127.0.0.1";
    }

    Server2Server *srv = init_server(remoteaddr, remotep, controlp, localp, mode); 
    
    if (srv == NULL)
    {
        fprintf(stderr, "Error starting server2server!\n");
        return -4; 
    }
    
    start_server(srv);
    return 0;
}