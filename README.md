# server2server-linux
The program that connects to firehop, so that you can hop over firewalls, on Linux. *server2server* connects two servers together: the server that is serving a service which you want to reverse and the *firehop* server on the accessible end. *firehop* is divided up into two programs so that the complexities of the programs are reduced, adhering to the UNIX philosophy of "one program for one task." 

A lot about *firehop* is explained on the respective *firehop* repository, so if you wish to know more in regards to the system of programs as a whole, you should visit the repository [here](https://github.com/emilarner/firehop-linux).

To explain the command-line arguments that are required for this program to run as it should, let us quote the help menu that is present within the program by default:

    server2server - Glues two servers together over the firewall, for firehop.
    Running the Linux/POSIX version.
    USAGE:
    server2server [args...]
    
    --remote, -r       [Required] Specifies the remote port to connect to.
    --local, -l        [Required] Specifies the local port to connect to.
    --control, -c      [Required] Specifies the control port to connect to.
    --address, -a      [Required] Specifies the address of the remote and control endpoints.
    --tcp, -t          [Default] Explicitly specifies TCP mode.
    --udp, -u          Specifies that the program should reverse a UDP service.
    --quiet, -q        Print absolutely nothing to the console.
    --help, -h         Print this menu, ceasing program operation afterwards.


The local port is the port that you wish to allow to be accessible over the firewall. The remote and control ports are the ports that are open on another server/computer that can be used to reverse the local port over the firewall.




