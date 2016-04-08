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



ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, deregistration);
	ATF_ADD_TEST_CASE(tcs, registration);
}
