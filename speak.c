/**
 ** speak.c  -  a server program that uses the socket interface to tcp (client)
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include "speak.h"

extern char *inet_ntoa( struct in_addr );

#define NAMESIZE		255
#define BUFSIZE			81

void client( int server_number, char *server_node ){
	int			length;
	int			n, len;
	short			fd;
	struct sockaddr_in	address;
	struct hostent		*node_ptr;
	char			local_node[NAMESIZE];
	char			buffer[BUFSIZE];

	/*  get the internet name of the local host node on which we are running  */
	if( gethostname( local_node, NAMESIZE ) < 0 ){
		perror( "client gethostname" );
		exit(1);
	}
	
	fprintf(stderr, "client running on node %s\n", local_node);

	/*  get the name of the remote host node on which we hope to find server  */
	if( server_node == NULL )
		server_node = local_node;
	fprintf(stderr, "client about to connect to server at port number %d on node %s\n",
			server_number, server_node);

	/*  get structure for remote host node on which server resides  */
	if( (node_ptr = gethostbyname( server_node )) == NULL ){
		perror( "client gethostbyname" );
		exit(1);
	}

	/*  set up Internet address structure for the server  */
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	memcpy(&address.sin_addr, node_ptr->h_addr, node_ptr->h_length);
	address.sin_port = htons(server_number);

	fprintf(stderr, "client full name of server node %s, internet address %s\n",
			node_ptr->h_name, inet_ntoa(address.sin_addr));

	/*  open an Internet tcp socket  */
	if( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		perror( "client socket" );
		exit(1);
	}

	/*  connect this socket to the server's Internet address  */
	if( connect( fd, (struct sockaddr *)&address, sizeof(address) ) < 0 ){
		perror( "client connect" );
		exit(1);
	}

	/*  now find out what local port number was assigned to this client  */
	len = sizeof(address);
	if( getsockname( fd, (struct sockaddr *)&address, &length ) < 0 ){
		perror( "client getsockname" );
		exit(1);
	}

	/*  we are now successfully connected to a remote server  */
	fprintf(stderr, "client at internet address %s, port %d\n",
			inet_ntoa(address.sin_addr), ntohs(address.sin_port));

	printf("====== You may talk to the server now! ======\n");

	time_t current_time;
    struct tm *local_time;
    char timestamp[20];

	/*  transmit data from standard input to server  */
    int current = 0; // 0-client, 1-server, default client
	int on = 1; // 1-true, 0-false
    while (1) {
		// client speaks, server listens
        while (current==0) {
			current_time = time(NULL);
			local_time = localtime(&current_time);
			strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", local_time);
			printf("\033[0;31m%s\033[0m Client: ", timestamp);
            if (fgets(buffer, BUFSIZE, stdin) == NULL) {
				perror("client fgets error");
			}

			len = strlen(buffer); 
			if( (n = send(fd, buffer, len, 0)) != len ) {
				if( n < 0 ){
					perror( "client send" );
					exit(1); // exit with error
				}
				else{
					fprintf(stderr, "client sent %d bytes, attempted %d\n", n, len);
				}
			}
			
			// receive acknowledgement from server
			n = recv(fd, buffer, sizeof(buffer), 0);
			printf("Received acknowledgment from server: %d bytes received\n", n);


            // == 0 is a match
            if (strcmp(buffer, "x\n")==0) {
				fprintf(stderr, "You've ended your turn\n");
                current = 1; // switch to server
            }

            else if (strcmp(buffer, "xx\n")==0) {
                fprintf(stderr, "====== Chat ended by client ======\n");
				on = 0;
                exit(0); // exit successfully to close connection 
            }
        }

		if (on == 0) break;
		// client listens 
		while (current==1) {
			current_time = time(NULL);
			local_time = localtime(&current_time);
			strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", local_time);
            // printf("Server's turn");
            n = recv( fd, buffer, BUFSIZE, 0);
			buffer[n] = '\0';
			if (n > 0) {
				send( fd, buffer, n, 0);
				if (strcmp(buffer, "x\n")==0) {
					fprintf(stderr, "Your turn now\n");
                	current = 0; // switch to client
				}
				else if (strcmp(buffer, "xx\n")==0) {
					fprintf(stderr, "====== Chat ended by server ======\n");
					on = 0;
					exit(0); // exit successfully to close connection 
				}
				else {
					fprintf(stdout, "\033[34m%s\033[0m Server: " , timestamp);
					fputs(buffer, stdout);
				}
			}
			else {
				perror( "client recv error" );
				exit(1); // exit with error
			}
            
        }
    }

	/*  close the connection to the server  */
	if( close(fd) < 0 ){
		perror( "client close" );
		exit(1);
	}
}