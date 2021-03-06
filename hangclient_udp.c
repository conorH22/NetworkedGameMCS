//Client program for hangman game
//conor holmes
//22/11/16

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include "Practical.h"

#define BUF_SIZE 500
#define LINESIZE 80
#define HANGMAN_TCP_PORT 1066

 int main(int argc, char *argv[])
   {
    struct addrinfo hints;//an addinfo struct
    struct addrinfo *result, *rp;//pointers to addrinfo structs
    int sfd, j, count; //s
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];
    char i_line[LINESIZE];
    char o_line[LINESIZE];
    char * srver_nme;

    
   if (argc < 3) {
            //fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
		DieWithUserMessage( "<Address/Name> <Port/Service>",argv[0]);
               exit(EXIT_FAILURE);
           }

           /* Obtain address(es) matching host/port */

           memset(&hints, 0, sizeof(struct addrinfo));
           hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
           hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
           hints.ai_flags = 0;
           hints.ai_protocol = 0;          /* Any protocol */

           srver_nme = getaddrinfo(argv[1], argv[2], &hints, &result);//note bene, passing an addrinfo struct and a ptr to an addrinfo struct by reference
           if (srver_nme != 0) {
               //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(srver_nme));
		DieWithSystemMessage("getaddrinfo() failed :%s\n");
               exit(EXIT_FAILURE);
           }

           /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully connect(2).
              If socket(2) (or connect(2)) fails, we (close the socket
              and) try the next address. */

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)//using connect with an UDP socket
                   break;                  /* Success */

               close(sfd);
           }
           //debug... learning
           printf("SD value = %d\n", sfd);
			printf ("Connected to host_info %s \n", srver_nme);
 	        while ((count = read (sfd, i_line, LINESIZE)) > 0) {
 		            write (1, i_line, count);
 		            count = read (0, o_line, LINESIZE);//0 = STDIN
 		            write (sfd, o_line, count);
 	           } 
           

           if (rp == NULL) {               /* No address succeeded */
               //fprintf(stderr, "Could not connect\n");
		DieWithSystemMessage("Can't connect()");
               exit(EXIT_FAILURE);
           }

           freeaddrinfo(result);           /* No longer needed */

           /* Send remaining command-line arguments as separate
              datagrams, and read responses from server */

           for (j = 3; j < argc; j++) {
               len = strlen(argv[j]) + 1;
                       /* +1 for terminating null byte */

               if (len + 1 > BUF_SIZE) {
                   /*fprintf(stderr,
                           "Ignoring long message in argument %d\n", j);*/
 		DieWithUserMessage("cant send remain cmd line args failed", gai_strerror(j));
                   continue;
               }

               if (write(sfd, argv[j], len) != len) {
                   fprintf(stderr, "partial/failed write\n");
		   DieWithSystemMessage("Partial/failed Write");
                   exit(EXIT_FAILURE);
               }
							 sleep(5);//debug(was10sec)... create a delay in order to get communicating pair info from lsof -i
            
               nread = read(sfd, buf, BUF_SIZE);
               if (nread == -1) {
		     DieWithSystemMessage("read() error");
                   //perror("read");
                   exit(EXIT_FAILURE);
               }

               printf("Received %zd bytes: %s\n", nread, buf);
           }

           exit(EXIT_SUCCESS);
       }

