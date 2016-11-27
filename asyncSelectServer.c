 /* Network server for hangman game */
 /* File: hangserver.c */
 /* Mark Douglas */

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 // #include <arpa/inet.h>

 extern time_t time ();

 int maxlives = 12;
 char *word [] = {
 # include "words"
 };
 # define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 # define MAXLEN 80 /* Maximum size in the world of Any string */
 # define HANGMAN_TCP_PORT 1066
 #define TRUE   1
 #define FALSE  0

 main ()
 {
	// printf("TEST!!");
 	int sock, fd, client_len;
	int optval, cliConnectedCount;
 	struct sockaddr_in server, client;
	fd_set readfds;
	char buf[MAXLEN];

 	srand ((int) time ((long *) 0)); /* randomize the seed */

 	sock = socket (AF_INET, SOCK_STREAM, 0);//0 or IPPROTO_TCP
	//This error checking is the code Stevens wraps in his Socket Function etc
 	if (sock <0) { 
 		perror ("creating stream socket");
 		exit (1);
 	}

	/* Us   */
	//optval = 1;	
	// setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));


	bzero((char *) &server, sizeof(server));	// Build the server's internet address
 	server.sin_family = AF_INET;	// Internet address
 	server.sin_addr.s_addr = htonl(INADDR_ANY);	// Get the system to translate the IP address
 	server.sin_port = htons(HANGMAN_TCP_PORT);	// Listen on the hangman port

	// Bind to the socket with the port
 	if (bind(sock, (struct sockaddr *) & server, sizeof(server)) <0) {
 		perror ("binding socket");
	 	exit (2);
 	}

	// Listen to connection requests and be ready to accept
 	listen (sock, 5);

	cliConnectedCount = 0;

 	while (TRUE) {
	printf("\nStarting processing client...");
 		client_len = sizeof (client);

		FD_ZERO(&readfds);		// Initialize the read fd
		FD_SET(sock, &readfds);	// Add the fd socket
		FD_SET(0, &readfds);	// Add the stdin for the fd
		// Select if the client has sent something
		if(select(sock+1, &readfds, 0, 0, 0) < 0)
		{
			error("Error in select()");
		}
		// Process the clients command if it has been sent
		if(FD_ISSET(0, &readfds))
		{
			printf("\nProcessing client command...");
			//fgets(buf, MAXLEN, stdin);
			play_hangman (sock, sock);
			close (sock); 	
			cliConnectedCount--;
		}
		// Process the clients connection request
		if(FD_ISSET(sock, &readfds))
		{	// Accept the connection
			printf("\nAccepting connection...");
			fd = accept (sock, (struct sockaddr *) &client, &client_len);
			if(fd < 0)
			{
				perror ("accepting connection failed");
 				exit (3);
			}
			printf("\nSuccess!! Readying hangman...");
			cliConnectedCount++;
				

		}
		
 		
 	}
 }

 /* ---------------- Play_hangman () ---------------------*/

 play_hangman (int in, int out)
 {
 	char * whole_word, part_word [MAXLEN],
 	guess[MAXLEN], outbuf [MAXLEN];

 	int lives = maxlives;
 	int game_state = 'I';//I = Incomplete
 	int i, good_guess, word_length;
 	char hostname[MAXLEN];

 	gethostname (hostname, MAXLEN);
 	sprintf(outbuf, "Playing hangman on host% s: \n \n", hostname);
	// CHANGE WRITE
 	send(out, outbuf, strlen (outbuf), 0);

 	/* Pick a word at random from the list */
 	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
 	syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

 	/* No letters are guessed Initially */
 	for (i = 0; i <word_length; i++)
 		part_word[i]='-';
 	
	part_word[i] = '\0';

 	sprintf (outbuf, "%s %d \n", part_word, lives);
	// CHANGE WRITE
 	send (out, outbuf, strlen(outbuf), 0);

 	while (game_state == 'I')
 	/* Get a letter from player guess */
 	{
		// CHANGE READ
		bzero(guess, MAXLEN);
		while (recv (in, guess, MAXLEN, 0) <0) {
 			if (errno != EINTR)
 				exit (4);
 			printf ("re-read the startin \n");
 			} /* Re-start read () if interrupted by signal */
 	good_guess = 0;
 	for (i = 0; i <word_length; i++) {
 		if (guess [0] == whole_word [i]) {
 		good_guess = 1;
 		part_word [i] = whole_word [i];
 		}
 	}
 	if (! good_guess) lives--;
 	if (strcmp (whole_word, part_word) == 0)
 		game_state = 'W'; /* W ==> User Won */
 	else if (lives == 0) {
 		game_state = 'L'; /* L ==> User Lost */
 		strcpy (part_word, whole_word); /* User Show the word */
 	}
 	sprintf (outbuf, "%s %d \n", part_word, lives);
	// CHANGE WRITE
 	send (out, outbuf, strlen (outbuf), 0);
 	}
	//printf("Exiting Server");
	//close(sock);
	//exit(0);
 }
