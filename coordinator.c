/* 
* Author : Yunus Dawji
* Student No : 210502433
* Email: cse03093@cse.yorku.ca
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

/*********************helper fucntion ****************/
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	
	int choice;
	//default value
	char *servername = "127.0.0.1";	
	char buffer[256];
	char **servers;
	int **ports;
	time_t t[4];


	start:
	// throw the menu output
	printf("\n---------------------------------------------------------------------\n");
	printf("Options: \n 1. SET_SERVER: set the address of attackers. \n 2. SET_TIMES: set time for each attacker \n"); 
	printf(" 3. SEND_TIMES: Send times to each attacker \n 4. Exit \n\n Enter your choice: ");

	// get the choice
	int tempcheck = scanf("%d", &choice);

	switch(choice)
	{
		// set ip address and port
		case 1:
			servers = (char **)malloc(4 * sizeof(char *));
			ports = (int **)malloc(4 * sizeof(int *));
			int i = 0;
			for(i = 0; i < 4; i++)
			{
				servers[i] = (char *) malloc(sizeof(buffer));
				ports[i] = (int *) malloc(sizeof(int));
				printf("Please enter address of attacker %d: ",i+1);
				bzero(buffer,256);
				scanf("%s",buffer);
				servers[i] = malloc(sizeof(buffer));			
				strncpy(servers[i], buffer, strlen(buffer)+1);	
				printf("Enter Port: ");
				scanf("%d",&ports[i][0]);
			}		
			goto start;
		// set ip address and port
		case 2:
		
			for( i = 0; i < 4; i++)
			{
				int h,m,s;
				printf("Please enter time in 24-hour format (hh:mm:ss) for server %d: ",i+1);
				scanf("%d:%d:%d",&h,&m,&s);
				time_t now;
				time(&now);

				struct tm * now_tm = localtime(&now);
				now_tm->tm_sec = s;
				now_tm->tm_min = m;
				now_tm->tm_hour = h;

				t[i] = mktime(now_tm);

			}
			goto start;
		// notify the servers
		case 3:
			
			for(i = 0; i < 4; i++){
				int sockfd, portno, n;
				struct sockaddr_in serv_addr;
				struct hostent *server;

				// open socket
				sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if (sockfd < 0) 
				error("ERROR opening socket");
				server = gethostbyname(servers[i]);
				if (server == NULL) {
					fprintf(stderr,"ERROR, no such host\n");
					exit(0);
				}
				bzero((char *) &serv_addr, sizeof(serv_addr));
				serv_addr.sin_family = AF_INET;
				bcopy((char *)server->h_addr,  (char *)&serv_addr.sin_addr.s_addr,server->h_length);
				serv_addr.sin_port = htons(ports[i][0]);
				if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 						error("ERROR connecting");
			
				char temptime[50];
				sprintf(temptime, "%ld",t[i]);
				
				n = write(sockfd,temptime,strlen(temptime));
				if (n < 0) 
					error("ERROR writing to socket");

				close(sockfd);
			}
			break;	
		default:
			break;
			
	}
	
	
	return 0;
}
