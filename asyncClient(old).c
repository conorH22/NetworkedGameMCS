/* Hangclient.c - Client for hangman server.  */
/* Mark Douglas */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>

#include <string.h>
#include <errno.h>

 # define LINESIZE 80
 # define HANGMAN_TCP_PORT 1066

 int main (int argc, char * argv [])
 {
 	struct sockaddr_in server; /* Server's address assembled here */
 	struct hostent * host_info;
 	int sock, count;
 	 char i_line[LINESIZE];
 	 char o_line[LINESIZE];
 	char * server_name;

	fd_set readset, writeset;
    struct timeval tv;
	int retval;

 	/* Get server name from the command line.  If none, use 'localhost' */

 	server_name = (argc = 1)?  argv [1]: "localhost";

 	/* Create the socket */
 	sock = socket (AF_INET, SOCK_STREAM, 0);
 	if (sock <0) {
 		perror ("Creating stream socket");
 		exit (1);
 	}

 	host_info = gethostbyname(server_name);
 	if (host_info == NULL) {
 		fprintf (stderr, "%s: unknown host:%s \ n", argv [0], server_name);
 		exit (2);
 	}

 	/* Set up the server's socket address, then connect */

 	server.sin_family = host_info->h_addrtype;
 	memcpy ((char *) & server.sin_addr, host_info->h_addr, host_info->h_length);
 	server.sin_port = htons (HANGMAN_TCP_PORT);

 	if (connect (sock, (struct sockaddr *) & server, sizeof server) <0) {
 		perror ("connecting to server");
 		exit (3);
 	}
 	/*OK connected to the server.  
 Take a line from the server and show it, take a line and send the user input to the server. 
 Repeat until the server terminates the connection. */

 	printf ("Connected to server% s \ n", server_name);
	//while(1){
		while(count = recv(sock, i_line, LINESIZE, NULL) > 0){
	 	//while ((count = read (sock, i_line, LINESIZE)) > 0) {
			
	 		//write (1, i_line, count);
	 		//count = read (0, o_line, LINESIZE);//0 = STDIN

			send(1, i_line, count);			
			count = recv(sock, o_line, LINESIZE, NULL);

			/* Watch stdin (fd 0) to see when it has input */
			FD_ZERO(&readset);
			FD_SET(0, &readset);
			/* Wait up to five seconds */
			tv.tv_sec = 5;
			tv.tv_usec = 0;

			retval = select(1, &readset, NULL, NULL, &tv);
			/* Don't rely on the value of tv now! */

			if(retval == -1)
				perror("select()");
			else if(retval)
				printf("Data is available now.\n");
				/* FD_ISSET(0, &readset) will be true */
			else
				printf("No data within five seconds");

	 		//write (sock, o_line, count);
			send(sock, o_line, count, NULL);
	 	}
	//}
 }
