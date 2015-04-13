/* 
* Author : Yunus Dawji
* Student No : 210502433
* Email: cse03093@cse.yorku.ca
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <inttypes.h>
#include <netinet/in.h>

/*********************Global Variables************************/
int sockfd, newsockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

int n;
FILE *students;

//used to put a lock when writing to a file
pthread_mutex_t lock;

//used for start a function
int choice;
int options; 

/******************function prototypes*****************************/
//thread handlers
void *processthread_handler(void *args);
void *socketthread_handler(void *arg);


//init helpers
void socket_init(int port);
void error(const char *msg);
int printRecords(FILE* records);
/*********************Helper functions************************/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/**
* Print records used by list option
*/   
int printRecords(FILE* records)
{
	char line[128];
	int verified = 0;
	printf("Students who answered:\n");
	while ( fgets ( line, sizeof line, records ) != NULL ) /* read a line */
	{
		printf("  %s", line); /* write the line */
	}
	printf("\n");
	return verified;        
}

/*********************Socket functions*****************************/
/**
* Logic process handle
* this is handler for each thread created to handle logic (authenticate 
* & accept their answer)
*/
void *processthread_handler(void *args)
{
	int newsockfdd = *(int *)args;
	char buffer[256], *address;
	time_t connecttime, disconnecttime;
	time(&connecttime);

	socklen_t fromlen;
	struct sockaddr addr;
	char ipstr[INET6_ADDRSTRLEN];
	
	
	
	// put it the records file //	
	FILE *fp;
	
	//reset the buffer for read
	//bzero(buffer,256);
	int n = recvfrom(newsockfdd,buffer,sizeof(buffer),0, &addr, &fromlen);
	if (n < 0) error("ERROR writing to socket");

	printf("%d", n);

	//copy the choice to temp variable
	address = malloc ( sizeof buffer );			
	//strncpy(buffer, address, strlen(buffer) + 1);

	// close the connection
	close(newsockfdd);
	time(&disconnecttime);
	
	char buf[80], buf1[80];
	strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", localtime(&connecttime));
	strftime(buf1, sizeof(buf1), "%a %Y-%m-%d %H:%M:%S %Z", localtime(&disconnecttime));
	
	struct sockaddr_in *tempaddr = (struct sockaddr_in *)&addr;
	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(tempaddr->sin_addr), ipAddress, INET_ADDRSTRLEN);


	//lock for writing
	pthread_mutex_lock(&lock);
	fp = fopen("conections.txt", "a");
	//write to file
	fprintf(fp, "%s \t%s \t%s\n",ipAddress,buf ,buf1);
	fclose(fp);
	//unlock after write
	pthread_mutex_unlock(&lock);
	
	//free(buffer);
	free(args);
}

/*********************Socket Server thread*******************/
//THINGS TO DO:
// 1. use args to set the port of server
// DO NOT FORGET
//
void *socketthread_handler(void *arg)
{

	//try to bind the socket to specific address else throw error
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
	      sizeof(serv_addr)) < 0) 
	      error("ERROR on binding");

	// sets the maximum number of client that can wait in queue to 
	// connect to socket 
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	

	while(1){
		//wait here untill a client connects
		newsockfd = accept(sockfd, 
			 (struct sockaddr *) &cli_addr, 
			 &clilen);
		if (newsockfd < 0) 
	  		error("ERROR on accept");
	
		

	  	
		//process handler or logic handler thread
		pthread_t processth;
		int *new_sock;
		new_sock = malloc(1);
		*new_sock = newsockfd;

		//create a thread with the socket		 
		if( pthread_create( &processth , NULL ,  processthread_handler , (void*) new_sock) < 0)
		{
		    perror("could not create thread");	   
		}
		 
		//wait for any pending connections
		//pthread_join( processth , NULL);
		
		
	}
	return NULL;
}

/********************init************************************/
void socket_init(int port) 
{
	//this creates new socket
	//first argument is address domain
	//second argument is data flow
	//thrid argument helps decide the protocol 
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
		error("ERROR opening socket");

	//set the server address struct to zeros(empty)
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//port number variable
	portno = port;

	//sets up the struct value
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
}

int main(int argc, char *argv[])
{
	

	pthread_t pth;	

	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}
	
	/********init**************/
	//socket init
	int port = 5000;
	socket_init(port);
	/**************************/

	//this just to clearout the file it exists at the start of program
	FILE *fp;
	fp = fopen("records.txt", "w+");
	fclose(fp);

	/**
	* main loop that genrates the menu
	*/
	while(1)
	{
		// throw the menu output
		printf("\n---------------------------------------------------------------------\n");
		printf("Options: \n 1. START_QUESTION: starts the server process that runs with n choices. \n 2. END_QUESTION(): Terminates the server process; students can \n    no longer send responses. \n"); 
		printf(" 3. LIST: Lists students who sent answers.\n 4. Exit \n \n Enter you choice ");

		// get the choice
		int tempcheck = scanf("%d", &choice);

		// if choice is start question
		if(choice == 1)
		{					
				
			//printf(" Enter the number of choices ");
			//tempcheck = scanf("%d", &options);	
			//int i = 0;

			//printf("\n\nSTARTING QUESTION\n\n");

			// Create thread that handles socket
			// this where the magic happens 
			pthread_create(&pth,NULL,socketthread_handler,"processing...");

		}
		else if (choice == 2)
		{
			//cancel the thread		
			pthread_cancel(pth);
			//close the socketfile handler
			close(newsockfd);
			//close the socket
			close(sockfd);
		}	
		else if (choice == 3)
		{
			//prints the name of students who answered
			FILE* temp = fopen("records.txt","r");
			printRecords(temp);
			fclose(temp);
		}
		else
		{
			if(sockfd)
				close(sockfd);
			return 0;	
		}	
	}
	return 0; 
}
