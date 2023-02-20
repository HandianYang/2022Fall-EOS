# Socket_補充

## 概念

<img src="https://i.imgur.com/WyXEYuY.png">

- `socket()` creates an endpoint for communication and returns a descriptor.
- `connect()` initiates a connection on a socket. Normally used in a client program for sending requests.
- `bind()` binds a name to a socket. Normally, listen() is invoked after binding to socket. Used in a server program.
- `listen()` waits for connections on a socket. Used in a server program to wait for incoming requests.
- `accept()` accepts a connection on a socket. Used in a server program.


## 實作

### sockop.h

```c
#ifndef _SOCKOP_H_
#define _SOCKOP_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define errexit(format, arg ...) exit(printf(format, ##arg))

/* Create server */
int passivesock(const char *service, const char *transport, int qlen);

/* Connect to server */
int connectsock(const char *host, const char *service, const char *transport);

#endif      /* _SOCKOP_H_ */
```

- `passivesock()` creates a socket and binds to the corresponding socket descriptor to passively accept the incoming connection requests.
- `connectsock()` creates a socket and initiates a connection to the remote host (specied in struct sockadd in sin).


### sockop.c

```c
#include "sockop.h"

/*
 * passivesock - allocate & bind a server socket using TCP or UDP
 * 
 * Arguments:
 *  service     - service associated with the desired port
 *  transport   - transport protocol to use ("tcp" or "udp")
 *  qlen        - maximum server request queue length
 */

int passivesock(const char *service, const char *transport, int qlen)
{
    struct servent *pse;        /* pointer to service information entry */
    struct sockaddr_in sin;     /* an Internet endpoint address */
    int s, type;                /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port));
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("Can't find \"%s\" service entry\n", service);
    
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0) 
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("Can't create socket: %s\n", strerror(errno));
    
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't bind to port %s: %s\n", service, strerror(errno));

    /* Set the maximum number of waiting connection */
    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        errexit("Can't listen on port %s: %s\n", service, strerror(errno));
    
    return s;
}


/*
 * connectsock  - allocate & connect a socket using TCP or UDP
 * 
 * Arguments:
 *  host        - name of host to which connection is desired
 *  service     - service associated with the desired port
 *  transport   - name of transport protocol to use ("tcp" or "udp")
 */
int connectsock(const char *host, const char *service, const char *transport)
{
    struct hostent *phe;        /* pointer to host information entry */
    struct servent *pse;        /* pointer to service information entry */
    struct sockaddr_in sin;     /* an Internet endpoint address */
    int s, type;                /* socket descriptor and socket type */

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    /* Map service name to port number */
    if ((pse = getservbyname(service, transport)))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
        errexit("Can't find \"%s\" service entry\n", service);
    
    /* Use protocol to choose a socket type */
    if (strcmp(transport, "udp") == 0) 
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        errexit("Can't create socket: %s\n", strerror(errno));
    
    /* Connect the socket */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("Can't connect to %s:%s : %s\n", host, service, strerror(errno));

    return s;
}
```


### server.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int sockfd, connfd;     /* socket descriptor */
    struct sockaddr_in addr_cln;
    socklen_t sLen = sizeof(addr_cln);
    int n;
    char snd[BUFSIZE], rcv[BUFSIZE];

    if (argc != 2)
        errexit("Usage: %s <port>\n", argv[0]);
    
    /* create socket and bind socket to port */
    sockfd = passivesock(argv[1], "tcp", 10);

    while (1) {
        /* waiting for connection */
        connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen);
        if (connfd == -1)
            errexit("Error: accept()\n");
        
        /* read message from client */
        if ((n = read(connfd, rcv, BUFSIZE)) == -1)
            errexit("Error: read()\n");
        
        printf("n = %d\n", n);
        
        /* write message back to the client */
        n = sprintf(snd, "Server: %.*s", n, rcv);
        printf("n = %d\n", n);
        if ((n = write(connfd, snd, n)) == -1)
            errexit("Error: write()\n");
        printf("n = %d\n", n);
        
        /* close client connection */
        close(connfd);
    }
    /* close server socket */
    close(sockfd);

    return 0;
}
```


### client.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sockop.h"

#define BUFSIZE 1024

int main(int argc, char *argv[])
{
    int connfd;         /* socket descriptor */
    int n;
    char buf[BUFSIZE];

    if (argc != 4)
        errexit("Usage: %s <host_address> <host_port> <message>\n", argv[0]);

    /* create socket and connect to server */
    connfd = connectsock(argv[1], argv[2], "tcp");

    /* write message to server */
    if ((n = write(connfd, argv[3], strlen(argv[3]))) == -1)
        errexit("Error: write()\n");
    
    /* read message from the server and print */
    memset(buf, 0, BUFSIZE);
    if ((n = read(connfd, buf, BUFSIZE)) == -1)
        errexit("Error: read()\n");
    printf("%s\n", buf);

    /* close client socket */
    close(connfd);
    
    return 0;
}
```