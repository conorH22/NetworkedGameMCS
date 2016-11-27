//UDP server for hangman game 
//Name:conor Holmes
//Date:22/11/16

 #include <sys/types.h>
 #include <stdio.h>		
 #include <stdlib.h>
 #include <syslog.h>
 #include <unistd.h>
 #include <string.h>
 #include <errno.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include "Practical.h"


 #define BUF_SIZE 500
 extern time_t time ();
 int maxlives = 12;
 char * word [] = {
	#include "words"
	};
 #define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 #define MAXLEN 80 /* Maximum size in the world of Any string */
 #define HANGMAN_UDP_PORT 1080

int main(int argc, char *argv[])
       {
           struct addrinfo hints;
           struct addrinfo *result, *rp;
           int sfd, s;
           struct sockaddr_storage peer_addr;
           socklen_t peer_addr_len;
           ssize_t nread;
           char buf[BUF_SIZE];

           if (argc != 2) {
               //fprintf(stderr, "Usage: %s port\n", argv[0]);
              
	         DieWithUserMessage( "<Port>", argv[0]);
			exit(EXIT_FAILURE);
           }

           memset(&hints, 0, sizeof(struct addrinfo));
           hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
           hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
           hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address on servers */
           hints.ai_protocol = 0;          /* Any protocol, only UDP in this instance. Author could have used IPPROTO_UDP */
           hints.ai_canonname = NULL;
           hints.ai_addr = NULL;
           hints.ai_next = NULL;

           s = getaddrinfo(NULL, argv[1], &hints, &result);//NB first arguement is NULL, no IP/ name provided, which is what we want on servers
           if (s != 0) {
               //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
				DieWithUserMessage("getaddrinfo() failed",gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully bind(2).
              If socket(2) (or bind(2)) fails, we (close the socket
              and) try the next address. */

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                       rp->ai_protocol);
               if (sfd == -1)
                   continue;

               if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                   break;                  /* Success */

               close(sfd);
           }
           
          printf("testing... SD val=%d\n", sfd);
			
			play_hangman(sfd, sfd);

           if (rp == NULL) {               /* No address succeeded */
              // fprintf(stderr, "Could not bind\n");
              // 
				DieWithSystemMessage("could not bind()");
				exit(EXIT_FAILURE);
           }

           freeaddrinfo(result);           /* No longer needed */

           /* Read datagrams and echo them back to sender */

           for (;;) {
               peer_addr_len = sizeof(struct sockaddr_storage);
               nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                       (struct sockaddr *) &peer_addr, &peer_addr_len);
               if (nread == -1)
                   continue;               /* Ignore failed request */

               char host[NI_MAXHOST], service[NI_MAXSERV];

               s = getnameinfo((struct sockaddr *) &peer_addr,
                               peer_addr_len, host, NI_MAXHOST,
                               service, NI_MAXSERV, NI_NUMERICSERV);
              if (s == 0)
                   printf("Received %zd bytes from %s:%s\n",
                           nread, host, service);
               else
                   //fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
					DieWithUserMessage("getnameinfo() failed",gai_strerror(s));

               if (sendto(sfd, buf, nread, 0,
                           (struct sockaddr *) &peer_addr,
                           peer_addr_len) != nread)
                  // fprintf(stderr, "Error sending response\n");
				DieWithSystemMessage("sendto() error");
           }
       }

play_hangman(int in, int out)
{
	char * all_word, part_word [MAXLEN],
	guess[MAXLEN], outbuf [MAXLEN];

	int lives = maxlives;
	int game_state = 'I';// I = Incomplete
	int i, good_guess, word_length;
	char hostname[MAXLEN];
	srand ((int) time ((long *) 0));

	gethostname (hostname, MAXLEN);
	sprintf(outbuf, "playing hangman on host %s: \n \n", hostname);
	write(out, outbuf, strlen(outbuf));

	/*pick a word at random from list */
	all_word = word[rand() % NUM_OF_WORDS];
	word_length = strlen(all_word);
	syslog (LOG_USER | LOG_INFO, "Server chose hangman word %s",all_word);

	/* No letters are guessed Initially */
	for (i = 0; i < word_length; i++)
		part_word[i]= '-';

	part_word[i] = '\0';

	sprintf (outbuf, "%s %d \n", part_word, lives);
	write (out, outbuf, strlen(outbuf));

	while ( game_state == 'I')
	/* Get a letter from player guess */
	{
		while( read (in, guess,MAXLEN) <0) {
			if(errno != EINTR)
				exit(4);
			printf("re-read the startin \n");
			} /* Re-start read() if interrupted by signal */
	good_guess = 0;
	for (i = 0; i <word_length; i++) {
 		if (guess [0] == all_word [i]) {
 		good_guess = 1;
 		part_word [i] = all_word [i];
 		}
 	}
 	if (! good_guess) lives--;
 	if (strcmp (all_word, part_word) == 0)
 		game_state = 'W'; /* W ==> User Won */
 	else if (lives == 0) {
 		game_state = 'L'; /* L ==> User Lost */
 		strcpy (part_word, all_word); /* User Show the word */
 	}
 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen (outbuf));
 	}

}

