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
int flags = 0;
pthread_t signalThread;

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

void check_queue(int flags){
		SLIST_FOREACH(current_application, &head, next_application){
			int pid = current_application->pid;
			printf("PID %d IS REGISTERED\n", pid);
			if(flags & 0b1000 && current_application->condition == SIGSEVERE){
				kill(pid,SIGTEST);
				printf("KILLED SEVERE: %d\n", pid);					}
			if(flags & 0b10 && current_application->condition == SIGMIN){
				kill(pid,SIGTEST);
				printf("KILLED MIN: %d\n", pid);
			}
			if(flags & 0b100 && current_application->condition == SIGPAGESNEEDED){
				kill(pid,SIGTEST);
				printf("KILLED PAGES NEEDED: %d\n", pid);
			}
			random_millisecond_sleep(0,1000);
		}
}

void check_flags(int flags){
		if (flags & 0b1000 || flags & 0b10000){
			suspend_applications();
			resume_applications();
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

void init()
{
	SLIST_INIT(&head);
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));

	pthread_create(&signalThread, 0, monitor_signals, (void*)0);
	int fd=0;
	fd = open("/dev/lowmem", O_RDWR | O_NONBLOCK);
	int kq=kqueue();
	EV_SET(&change[0],fd,EVFILT_READ, EV_ADD,0,0,0);
}

void setup_flags(){
		printf("BLOCKING\n");
		int n=kevent(kq,change,1,event,1,NULL);
		printf("UNBLOCKING\n");
		flags = 0;
		flags = event[0].data;
		printf("DATA: %d\n", flags);
}

void sleepFor(int seconds){
		struct timespec sleepFor;
		sleepFor.tv_sec = seconds;
		sleepFor.tv_nsec = 0;
		nanosleep(&sleepFor, 0);
}
