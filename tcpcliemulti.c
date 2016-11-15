/* HangClient using fork host_info. */
/* conor Holmes*/
/* Date: 9/10/16*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
 #include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
# define LINESIZE 80
# define HANGMAN_TCP_PORT 1066

int main(int argc, char *argv[]) {
   int sockfd, portno, n, count;
   struct sockaddr_in serv_addr;
   struct hostent *host_info;
   char i_line[LINESIZE];
   char o_line[LINESIZE];	
   char * srver_nme;
   
   char buffer[256];
  
    srver_nme = (argc = 1)?  argv [1]: "localhost";
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   host_info = gethostbyname(srver_nme);
   
   if (host_info == NULL) {
      fprintf(stderr," %s ERROR,no such host %s \n", argv [0], srver_nme);
      exit(2);
   }
   /* Set up the server's socket address, then connect */
   /* Bzero is used instead memset.Bcopy: copies byte sequence*/		
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = host_info-> h_addrtype;
   bcopy((char *)host_info->h_addr, (char *)&serv_addr.sin_addr.s_addr, host_info->h_length);
   serv_addr.sin_port = htons(HANGMAN_TCP_PORT);
   
   /* Now connect to the host_info */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(3);
   }
     /*OK connected to the server.  
 Take a line from the host_info and show it, take a line and send the user input to the server. 
 Repeat until the server terminates the connection. */

 	printf ("Connected to host_info %s \n", srver_nme);
 	while ((count = read (sockfd, i_line, LINESIZE)) > 0) {
 		write (1, i_line, count);
 		count = read (0, o_line, LINESIZE);//0 = STDIN
 		write (sockfd, o_line, count);
 	} 

}
