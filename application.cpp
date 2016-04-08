#include <iostream>
#include <vector>

#include <fcntl.h> //Some C libraries are required
#include <kvm.h>
#include <libutil.h>
#include <signal.h>
#include <stdlib.h>
#include <fstream>

#include <sys/event.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <vm/vm_param.h>

#define SIGTEST 44

#define SIGSEVERE 45
#define SIGMIN 46
#define SIGPAGESNEEDED 47 
#define SIGREGISTERED 48 
#define SIGDEREGISTERED 49 

static SLIST_HEAD(slisthead, managed_application) head = SLIST_HEAD_INITIALIZER(head);
static struct slisthead *headp;

struct managed_application
{
	int pid, condition;
	SLIST_ENTRY(managed_application) next_application;
};

struct memStatus
{
	bool target, min, needed, severe, swap_low;
};

using namespace std; 

int isRegistered(int pid){

	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));
	struct managed_application *np_temp = (managed_application*)malloc(sizeof(struct managed_application));
	if (SLIST_FIRST(&head) != NULL){
		SLIST_FOREACH_SAFE(current_application, &head, next_application, np_temp){
			if (current_application->pid == pid){
				std::cout << "in queue" << endl;	
				return true;				
			}
		}
	}
	std::cout << "not in queue " << endl;
	return false;	
}

void random_millisecond_sleep(int min, int max)
{
	struct timespec sleepFor;
	int randomMilliseconds = ((rand() % max)+min) * 1000 * 1000;
	sleepFor.tv_sec = 0;
	sleepFor.tv_nsec = randomMilliseconds;
	nanosleep(&sleepFor, 0);
}

void monitor_application(int signal_number, siginfo_t *info, void *unused){
	
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));
	struct managed_application *np_temp = (managed_application*)malloc(sizeof(struct managed_application));
	struct managed_application *application = (managed_application*)malloc(sizeof(struct managed_application));
	
	if (SLIST_FIRST(&head) != NULL){
		SLIST_FOREACH_SAFE(current_application, &head, next_application, np_temp){
			if (current_application->pid == info->si_pid){
				SLIST_REMOVE(&head, current_application, managed_application, next_application);
	
				free(current_application);
				printf("DEREGISTERED\n");
				return;
			}
			if (kill(current_application->pid,0)==-1){
				SLIST_REMOVE(&head, current_application, managed_application, next_application);
				free(current_application);
				printf("TIMED OUT\n");
			}
		}
	}
	application->pid = info->si_pid;	
	application->condition = signal_number;
	SLIST_INSERT_HEAD(&head, application, next_application);
	printf("REGISTERED\n");

}

void suspend_applications()
{
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));
	SLIST_FOREACH(current_application, &head, next_application){
		int pid = current_application->pid;
		kill(pid, SIGSTOP);
		printf("SUSPENDED: %d\n", pid);
	}
}

void resume_applications()
{
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));
	SLIST_FOREACH(current_application, &head, next_application){
		int pid = current_application->pid;
		kill(pid, SIGCONT);
		printf("CONTINUED: %d\n", pid);
		random_millisecond_sleep(0,1000);
	}
}

//Query the device for updates statuses. 
memStatus queryDev()
{
	memStatus status = {false,false,false,false,false};
	// Read the file, C++ libraries are no good for reading from a device
	int devfile = open("/dev/lowmem", O_RDWR | O_NONBLOCK);
	if(devfile >= 0){
		char buf[5+sizeof(int)];
		int bytesRead=0;
		int swap_pages=0;
		int swap_space=0;
		//If the transfer worked
		
		if ((bytesRead = read(devfile,&buf,100))) {
		    if(buf[0] & 0b1)
		    	status.target=true;
		    if(buf[0] & 0b10)
		    	status.min=true;
		    if(buf[0] & 0b100)
		    	status.needed=true;
		    if(buf[0] & 0b1000)
		    	status.severe=true;
		    memcpy(&swap_pages, &buf[1], sizeof(int));
		    swap_space = swap_pages * getpagesize();
		    //printf("swap_space: %d\n", swap_space);
		    if(swap_space<250000000){
			status.swap_low=true;
			printf("LOW SWAP!!\n");
			}
		}
	}
	return status;
}

void check_applications(bool severe, bool min, bool needed){
	
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));

	SLIST_FOREACH(current_application, &head, next_application){
		int pid = current_application->pid;
		if(severe && current_application->condition == SIGSEVERE){
			kill(pid,SIGTEST);
			printf("KILLED SEVERE: %d\n", pid);
		}
		if(min && current_application->condition == SIGMIN){
			kill(pid,SIGTEST);
			printf("KILLED MIN: %d\n", pid);
		}
		if(needed && current_application->condition == SIGPAGESNEEDED){
			kill(pid,SIGTEST);
			printf("KILLED PAGES NEEDED: %d\n", pid);
		}
		random_millisecond_sleep(0,1000);
	}
	
}

void init_monitoring()
{
	struct sigaction sig;
	sig.sa_sigaction = monitor_application;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGSEVERE, &sig, NULL);
	sigaction(SIGMIN, &sig, NULL);
	sigaction(SIGPAGESNEEDED, &sig, NULL);

	SLIST_INIT(&head);
}
