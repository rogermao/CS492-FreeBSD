#include <atf-c.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define SIGTEST 44
#define SIGSEVERE 45
#define SIGMIN 46
#define SIGPAGESNEEDED 47
#define SIGREGISTERED 48

volatile int registered = 0;
sem_t sem;

static void receiveRegistrationSig(int n, siginfo_t *info, void *unused){
	printf("registration signal received");
	sem_wait(&sem);
	registered = 1;
	sem_post(&sem);
}

ATF_TC(registration);
ATF_TC_HEAD(registration, tc)
{
	atf_tc_set_md_var(tc, "descr", "This tests registration of applications");
}
ATF_TC_BODY(registration, tc)
{
	sem_init(&sem,1,1);	
	FILE *in = popen("pgrep daemon","r");
	int pid;
	char line[20];
	fgets(line, 20, in);
	sscanf(line, "%d", &pid);
	printf("daeon pid: %d", pid);

	struct sigaction sig;
	sig.sa_sigaction = receiveRegistrationSig;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGREGISTERED, &sig, NULL);

	kill(pid, SIGSEVERE);

	usleep(3000000);

	if(registered){
		atf_tc_pass();
	}
	else{
		atf_tc_fail("Not Registered");
	}	
}

ATF_TC(deregistration);
ATF_TC_HEAD(deregistration, tc)
{
	atf_tc_set_md_var(tc, "descr", "This tests deregistration of applications");
}
ATF_TC_BODY(deregistration, tc)
{
	sem_init(&sem,1,1);	
	FILE *in = popen("pgrep daemon","r");
	int pid;
	char line[20];
	fgets(line, 20, in);
	sscanf(line, "%d", &pid);
	printf("daemon pid: %d", pid);

	struct sigaction sig;
	sig.sa_sigaction = receiveRegistrationSig;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGREGISTERED, &sig, NULL);

	kill(pid, SIGSEVERE);

	//Todo
	//Add deregistration signal
	//Add callback function to receive deregistration signal	
	
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, registration);

	return atf_no_error();
}
