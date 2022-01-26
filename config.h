#ifndef CONFIG_H
#define CONFIG_H

#include <sys/select.h>

/* You should probably leave this alone, unless you know what you're doing. */ 

#define VERSION "1.0b"
#define RECV_BUFFER_SIZE 4096
#define SELECT_TIMEOUT_SEC 900

/* Number of file descriptors select() will scan for. Note: this cannot go above 1024 */
/* Also: the more select() will scan for, the longer it will take! */ 
#define SELECT_FDS 256


/* The strftime() format for the log printing. */
#define TIME_FORMAT "%m/%d/%y %H:%M"

/* Warning. */
#if SELECT_FDS >= FD_SETSIZE
    #error "select() cannot handle file descriptors greater than or equal to 1024"
#endif

#endif