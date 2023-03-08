/**
 ** speakd.c  -  a server program that uses the socket interface to tcp (server) 
 **
 **/

#include <stdio.h> // input/output library
#include <stdlib.h> // exit, memset
#include <string.h> // strlen, memcpy
#include <unistd.h> // UNIX standard library: gethostname, close
#include <sys/socket.h> // socket library: socket, bind
#include <netdb.h> // network database library: gethostbyname
#include <netinet/in.h> //  Internet Protocol library: htons, ntohs
#include <time.h>
#include "speakd.h"

extern char *inet_ntoa( struct in_addr );

#define NAMESIZE		255
#define BUFSIZE			81
#define listening_depth		2

void server( int server_number ){
	int			c, i;
	int			n, len;
	short			fd, client_fd; // store file descriptor for server/client socket
	struct sockaddr_in	address, client; // struct sockaddr_in is from <netinet/in.h>,  Internet socket address
	struct hostent		*node_ptr; // struct hostent is from <netdb.h>, hold information about a particular host on a network(name, address, etc.)
	char			local_node[NAMESIZE]; // string-> char xx[] or char*
	char			buffer[BUFSIZE+1]; // store the string typed in command line

	

	/*  get the internet name of the local host node on which we are running  */
	if( gethostname( local_node, NAMESIZE ) < 0 ){
		perror( "server gethostname" ); // from <stdio.h>, print an error message to the standard error (stderr) stream
		exit(1);
	}
	fprintf(stderr, "server running on node %s\n", local_node);

	/*  get structure for local host node on which server resides  */
	if( (node_ptr = gethostbyname( local_node )) == NULL ){
		perror( "server gethostbyname" );
		exit(1);
	}

	/*  set up Internet address structure for the server  */
	// from <string.h>, a pointer to the starting address of the memory block, the value to fill the memory with, and the number of bytes to be filled
	// fill the memory occupied by the address struct with the value of zero
	memset(&address, 0, sizeof(address)); 
	address.sin_family = AF_INET;
	// set address.sin_addr
	// COPY the IP address stored in h_addr field of node_ptr(hold network info) TO sin_addr field of address(Internet socket address)
	memcpy(&address.sin_addr, node_ptr->h_addr, node_ptr->h_length);
	address.sin_port = htons(server_number);

	fprintf(stderr, "server full name of server node %s, internet address %s\n",
			node_ptr->h_name, inet_ntoa(address.sin_addr)); // address.sin_addr -> IP address

	/*  open an Internet tcp socket  */
	// creates a new socket, AF_INET -> internet socket, SOCK_STREAM -> TCP socket, 0 -> use the default protocol (TCP in this case)
	// If the socket function fails it will return a value less than 0
	if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror( "server socket" );
		exit(1);
	}

	/*  bind this socket to the server's Internet address  */
	// bind() is from <sys/socket.h>, int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	// (struct sockaddr *)&address -> a pointer to the socket address structure that contains the information about the address and port
	// struct sockaddr_in address is a specific type of struct sockaddr
	// '&' -> passes the address of the variable address in stead of the struct itself
	if( bind( fd, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
		perror( "server bind" );
		exit(1);
	}

	/*  now find out what local port number was assigned to this server  */
	len = sizeof(address);
	if( getsockname( fd, (struct sockaddr *)&address, &len ) < 0 ){
		perror( "server getsockname" );
		exit(1);
	}

	/*  we are now successfully established as a server  */
	fprintf(stderr, "server at internet address %s, port %d\n",
			inet_ntoa(address.sin_addr), ntohs(address.sin_port));

	/*  start listening for connect requests from clients  */
	if( listen( fd, listening_depth ) < 0 ){
		perror( "server listen" );
		exit(1);
	}

	/*  now accept a client connection (we'll block until one arrives)  */
	len = sizeof(client);
	if( (client_fd = accept(fd, (struct sockaddr *)&client, &len)) < 0 ){
		perror( "server accept" );
		exit(1);
	}

	/*  we are now successfully connected to a remote client  */
	fprintf(stderr, "server connected to client at Internet address %s, port %d\n",
			inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    

	time_t current_time;
    struct tm *local_time;
    char timestamp[20];
    
	int current = 0; // 0-client, 1-server, client starts first so default to 0
	int on = 1; // 1-true, 0-false

    while (1) {
		// server listens 
		while (current==0) {
			current_time = time(NULL);
			local_time = localtime(&current_time);
			strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", local_time);
            n = recv( client_fd, buffer, BUFSIZE, 0);
			buffer[n] = '\0';
			if (n > 0) {
				send(client_fd, buffer, n, 0);
				if (strcmp(buffer, "x\n")==0) {
					fprintf(stderr, "Your turn now\n");
					current = 1; // switch to server
				}
				else if (strcmp(buffer, "xx\n")==0) {
					fprintf(stderr, "====== Chat ended by client ======\n");
					on = 0;
					exit(0); // exit successfully to close connection 
				}
				else {
					// fputs("%s Client: " , timestamp, stdout);
					fprintf(stdout, "\033[0;31m%s\033[0m Client: " , timestamp);
					fputs(buffer, stdout);
				}	
			}
			else {
				perror( "server recv error" );
				exit(1); // exit with error
			}
        }

		if (on == 0) break;
		// server speaks
        while (current==1) {
			// empty buffer here
			current_time = time(NULL);
			local_time = localtime(&current_time);
			strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", local_time);

			printf("\033[34m%s\033[0m Server: ", timestamp);
			if (fgets(buffer, BUFSIZE, stdin) == NULL) {
				perror("client fgets error");
			}
            len = strlen(buffer); 
            if( (n = send(client_fd, buffer, len, 0)) != len ) {
                if( n < 0 ){
                    perror( "server send" );
                    exit(1); // exit with error
                }
                else{
                    fprintf(stderr, "server sent %d bytes, attempted %d\n", n, len);
                }
            }

			// receive acknowledgement from client
			n = recv(client_fd, buffer, sizeof(buffer), 0);
			printf("Received acknowledgment from client: %d bytes received\n", n);

            // == 0 is a match
            if (strcmp(buffer, "x\n")==0) {
				fprintf(stderr, "Your turn has ended\n");
                current = 0; // switch to client
            }
            else if (strcmp(buffer, "xx\n")==0) {
                fprintf(stderr, "====== Chat ended by server ======\n");
				on = 0;
                exit(0); // exit successfully to close connection 
            }
        }

    }

   	/*  close the connection to the client  */
	if( close(client_fd) < 0 ){
		perror( "server close connection to client" );
		exit(1);
	}

	/*  close the "listening post" socket by which server made connections  */
	if( close(fd) < 0 ){
		perror( "server close" );
		exit(1);
	}

}
