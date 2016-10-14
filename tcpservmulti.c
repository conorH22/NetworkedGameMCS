/* Update server with fork for hangman game*/
/* name :conor Holmes*/
/* Date:08/10/2016*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>

extern time_t time ();

 int maxlives = 12;
 char *word [] = {
 #include "words"
 };

# define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
# define MAXLEN 80 /* Maximum size in the world of Any string */
# define HANGMAN_TCP_PORT 1066



 main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   int n, pid;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   srand ((int) time ((long *) 0)); /* randomize the seed */
   
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   /*portno = 5001;*/
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(HANGMAN_TCP_PORT);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(2);
   }
   
   /* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
	 play_hangman(newsockfd, newsockfd);
         exit(3);
      }
      else {
	 
         close(newsockfd);
      }
		
   } /* end of while */
}

play_hangman(int in, int out)
{
	char * all_word, part_word [MAXLEN],
	guess[MAXLEN], outbuf [MAXLEN];

	int lives = maxlives;
	int game_state = 'I';// I = Incomplete
	int i, good_guess, word_length;
	char hostname[MAXLEN];

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