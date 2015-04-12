#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)


//variable for timer posix api
timer_t timerid; // timer id
struct sigevent sev; //
struct itimerspec its; // specs (interval)
long long freq_nanosecs; 
sigset_t mask;
struct sigaction sa;
int i = 0;
static void handler(int sig, siginfo_t *si, void *uc)
{
    if(si->si_value.sival_ptr != &timerid){
        printf("Stray signal\n");
    } else {
	i++;
        printf("Caught signal %d from timer\n", sig);
    }
}

int main(int args, char *argv[])
{

	//set handler for the timer
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIG, &sa, NULL) == -1)
		errExit("sigprocmask");

	//create the timer
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIG;
	sev.sigev_value.sival_ptr = &timerid;
	if(timer_create(CLOCKID, &sev, &timerid) == -1)
		errExit("timer_create");


	//set the specs
	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if(timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("timer_settime");

		
	while(i < 10){
		//sleep(100);
		if(i == 9)
			timer_delete(timerid);
	}
	exit(EXIT_SUCCESS);

}

