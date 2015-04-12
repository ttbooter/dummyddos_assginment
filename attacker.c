#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

/********************Global Defines***************************/
#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)

/*********************Global Variables************************/
int sockfd, newsockfd, portno;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

//variables for timer posix api
timer_t timerid; // timer id
struct sigevent sev; 
struct itimerspec its; // specs (interval)
sigset_t mask;
struct sigaction sa;

//threads
pthread_t attackth;


/******************function prototypes*****************************/
//thread handlers
void *processthread_handler(void *args);
void *socketthread_handler(void *arg);
void *attackthread_handler(void *args);

//timer handler
static void timer_handler(int sig, siginfo_t *si, void *uc);

//init helpers
void timer_init();
void socket_init(int port);

/********************timer handler*********************************/
static void timer_handler(int sig, siginfo_t *si, void *uc)
{
	if(si->si_value.sival_ptr != &timerid){
	printf("Stray signal\n");
	} else {


	// Create thread that handles socket
	// this where the magic happens 
	pthread_create(&attackth,NULL,attackthread_handler,"processing...");

	printf("Caught signal %d from timer\n", sig);
	}
}

/*********************Attack Functions*****************************/
void *attackthread_handler(void *args)
{
	//connect to server and disconnect
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
	
	char *buffer;
	
	//temp variables to store the student number and choice
	char *studentnumber, *schoice;
	long timeofattack;
	
	buffer = (char *)malloc(sizeof(char)*256);
	
	// reset the buffer for the read 
	bzero(buffer,256);

	// read the timestamp for coordinator
	int n = read(newsockfdd,buffer,255);
	if (n < 0) error("ERROR reading from socket");

	//copy the student number to temp variable
	timeofattack = strtol (buffer, NULL, 10);
	
	long currenttime = (long) time(NULL);

	if(timeofattack < currenttime)
	{
		error("ERROR Invalid Time of Attack");
		close(newsockfdd);	
	}
	else if(timeofattack >= currenttime)
	{
		//start the timer which start the client thread
		//set the specs
		its.it_value.tv_sec = timeofattack - currenttime;
		its.it_value.tv_nsec = 0;
		its.it_interval.tv_sec = its.it_value.tv_sec;
		its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
		//activate the timer
		if(timer_settime(timerid, 0, &its, NULL) == -1)
			errExit("timer_settime");
			
	}

	//for debug********************************/
	printf("Time of attack: %ld\n",timeofattack);
	//*****************************************/
		
	// close the connection
	close(newsockfdd);
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
		pthread_join( processth , NULL);
		
		
	}
	return NULL;
}

/********************init************************************/
void timer_init()
{
	//set handler for the timer
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIG, &sa, NULL) == -1)
		errExit("sigprocmask");

	//create the timer
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if(timer_create(CLOCKID, &sev, &timerid) == -1)
		errExit("timer_create");


	//set default the specs
	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
}

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

/*********************Main***********************************/
int main(int argc, char *argv[])
{
	

	pthread_t pth;	

	if (argc < 2) {
	 fprintf(stderr,"ERROR, no port provided\n");
	 exit(1);
	}

	/********init**************/
	//timer init
	timer_init();
	//socket init
	int port = 5000;
	socket_init(port);
	/**************************/
	
	//this just to clearout the file it exists at the start of program
	FILE *fp;
	fp = fopen("records.txt", "w+");
	fclose(fp);

	int choice;

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
			//printRecords(temp);
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
