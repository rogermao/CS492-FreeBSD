#include <atf-c++.hpp>

#include "application.cpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define SIGTEST 44
#define SIGSEVERE 45
#define SIGMIN 46

volatile int received = 0;

static int getDaemonPID(){
	FILE *in = popen("pgrep daemon","r");
	int pid = 0;
	fscanf(in, "%d", &pid);
	
	std::cout << "daemon pid" << pid;

	return pid;
}

void receiveSignal(int n, siginfo_t *info, void *unused){
	received = 1;	
}

void setupHandler(int signal){
	struct sigaction sig;
	sig.sa_sigaction = receiveSignal;
	sig.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sigaction(signal, &sig, NULL); 
}

int registerApp(int pid, int signal){

	siginfo_t *info;
	info->si_pid = pid;
	void* unused;	

	monitor_application(signal, info, unused);

	int result = isRegistered(pid);

	return result;
}

void sleepFor(int secs){
	struct timespec sleepInterval; 
	sleepInterval.tv_sec = secs;
	sleepInterval.tv_nsec = 0; 

	nanosleep(&sleepInterval, 0); 
}
