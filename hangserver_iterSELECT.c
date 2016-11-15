 /* Network server for hangman game */
 /* File: hangserver.c */

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <stdio.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>
 #include <string.h>   //strlen
 #include <stdlib.h>
 #include <unistd.h>   //close
 #include <arpa/inet.h>    //close
 #include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

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
 	int sock, fd, client_len;
	int opt = TRUE;
        int master_socket , addrlen , new_socket , client_socket[30] , 	max_clients = 30 , activity, i , valread , sd;
    	int max_sd;
    	struct sockaddr_in address;
 	struct sockaddr_in server, client;

 	srand ((int) time ((long *) 0)); /* randomize the seed */

	    char buffer[1025];  //data buffer of 1K
      
	    //set of socket descriptors
	    fd_set readfds;

 	sock = socket (AF_INET, SOCK_STREAM, 0);//0 or IPPROTO_TCP
 	if (sock <0) { //This error checking is the code Stevens wraps in his Socket Function etc
 		perror ("creating stream socket");
 		exit (1);
 	}

 	server.sin_family = AF_INET;
 	server.sin_addr.s_addr = htonl(INADDR_ANY);
 	server.sin_port = htons(HANGMAN_TCP_PORT);

 	if (bind(sock, (struct sockaddr *) & server, sizeof(server)) <0) {
 		perror ("binding socket");
	 	exit (2);
 	}

 	listen (sock, 5);

 	while (1) {
 		client_len = sizeof (client);
 		if ((fd = accept (sock, (struct sockaddr *) &client, &client_len)) <0) {
 			perror ("accepting connection");
 			exit (3);
 		}
 		play_hangman (fd, fd);
 		close (fd);
 	}
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
 //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
      
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
	 //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++) 
    {
        client_socket[i] = 0;
    }
      
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
//else its some IO operation on some other socket :)
        for (i = 0; i < max_clients; i++) 
        {
            sd = client_socket[i];
              
            if (FD_ISSET( sd , &readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                      
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                  
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
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
 	write(out, outbuf, strlen (outbuf));

 	/* Pick a word at random from the list */
 	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
 	syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

 	/* No letters are guessed Initially */
 	for (i = 0; i <word_length; i++)
 		part_word[i]='-';
 	
	part_word[i] = '\0';

 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen(outbuf));

 	while (game_state == 'I')
 	/* Get a letter from player guess */
 	{
		while (read (in, guess, MAXLEN) <0) {
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
 	write (out, outbuf, strlen (outbuf));
 	}
 }
