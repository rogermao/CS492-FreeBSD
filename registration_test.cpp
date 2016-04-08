#include <atf-c++.hpp>

#include "application.cpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#define SIGTEST 44
#define SIGSEVERE 45
#define SIGMIN 46

volatile int registered = 0;
static int getDaemonPID(){
	FILE *in = popen("pgrep daemon","r");
	int pid = 0;
	fscanf(in, "%d", &pid);
	
	std::cout << "daemon pid" << pid;

	return pid;
}

ATF_TEST_CASE(deregistration);
ATF_TEST_CASE_HEAD(deregistration)
{
	set_md_var("descr", "This tests deregistration of applications");
}
ATF_TEST_CASE_BODY(deregistration)
{
	int pid = getDaemonPID();

	siginfo_t *info;
	info->si_pid = 999;
	void* unused;	

	monitor_application(SIGSEVERE, info, unused);

	int result = isRegistered(999);

	if(result == 0){
		ATF_FAIL("Registration Failed");	
	}	

	monitor_application(SIGSEVERE, info, unused);

	result = isRegistered(999);

	if(!result){
		ATF_PASS();
	}
	else{
		ATF_FAIL("Not Deregistered");
	}	
}

ATF_TEST_CASE(registration);
ATF_TEST_CASE_HEAD(registration)
{
	set_md_var("descr", "This tests registration of applications");
}
ATF_TEST_CASE_BODY(registration)
{

	int pid = getDaemonPID();

	siginfo_t *info;
	info->si_pid = 999;
	void* unused;	

	monitor_application(SIGSEVERE, info, unused);

	int result = isRegistered(999);

	if(result == 0){
		ATF_FAIL("Registration Failed");	
	}else{
		ATF_PASS();
	}
}

ATF_TEST_CASE(sigmin);
ATF_TEST_CASE_HEAD(sigmin)
{
	set_md_var("descr", "This test if sigmin signal triggered 
properly");
}
ATF_TEST_CASE_BODY(sigmin)
{

	kill(pid, SIGMIN);
	
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGTEST, &sig, NULL); 
	
	srand(time(NULL));
	int *mem;
	int memsize=1024; 
	mem=malloc(memsize);
	
	while(!pausevar){
		mem=realloc(mem,memsize=memsize+1024*1024*10);
		sleep(1);
	}
	
	mem = realloc(mem,memsize = memsize/8);
	sem_wait(&sem);
	pausevar=0;
	sem_post(&sem);
	
	ATF_PASS();
	
	ATF_FAIL("THIS SHOULD NEVER FAIL");

}
ATF_TEST_CASE(sigpagesneeded);
ATF_TEST_CASE_HEAD(sigpagesneeded)
{
	set_md_var("descr", "This test if sigpagesneeded signal 
triggered properly");
}
ATF_TEST_CASE_BODY(sigpagesneeded)
{

	kill(pid, SIGMIN);
	
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGTEST, &sig, NULL); 
	
	srand(time(NULL));
	int *mem;
	int memsize=1024; 
	mem=malloc(memsize);
	
	while(!pausevar){
		mem=realloc(mem,memsize=memsize+1024*1024*10);
		sleep(1);
	}
	
	mem = realloc(mem,memsize = memsize/8);
	sem_wait(&sem);
	pausevar=0;
	sem_post(&sem);
	
	ATF_PASS();
	
	ATF_FAIL("THIS SHOULD NEVER FAIL");

}
ATF_TEST_CASE(sigstop);
ATF_TEST_CASE_HEAD(sigstop)
{
	set_md_var("descr", "This test if sigmin signal triggered 
properly");
}
ATF_TEST_CASE_BODY(sigstop)
{

	kill(pid,SIGMIN);
	
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGSTOP, &sig, NULL); 
	
	while(i < timeoutCount){
		nanosleep(&sleepInterval, 0); 
		if(pausevar==1){
			ATF_PASS();
		}
	}
	
	ATF_FAIL("THIS SHOULD NEVER FAIL");

}
ATF_TEST_CASE(sigcont);
ATF_TEST_CASE_HEAD(sigcont)
{
	set_md_var("descr", "This test if sigmin signal triggered 
properly");
}
ATF_TEST_CASE_BODY(sigcont)
{

	kill(pid,SIGMIN);
	
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGCONT, &sig, NULL); 
	
	int i = 0;
	struct timespec sleepInterval; 
	sleepInterval.tv_sec = 1;
	sleepInterval.tv_nsec = 0; 
	int timeoutCount = 10; 
	
	while(i < timeoutCount){
		nanosleep(&sleepInterval, 0); 
		if(pausevar==1){
			ATF_PASS();
		}
	}
	
	ATF_FAIL("THIS SHOULD NEVER FAIL");

}
ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, deregistration);
	ATF_ADD_TEST_CASE(tcs, registration);
	ATF_ADD_TEST_CASE(tcs, sigmin);
	ATF_ADD_TEST_CASE(tcs, sigpagesneeded);
	ATF_ADD_TEST_CASE(tcs, sigstop);
	ATF_ADD_TEST_CASE(tcs, sigcont);
}
