#include <atf-c++.hpp>

#include "test_util.cpp"

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

ATF_TEST_CASE(registration);
ATF_TEST_CASE_HEAD(registration)
{
	set_md_var("descr", "This tests registration of applications");
}
ATF_TEST_CASE_BODY(registration)
{
	int app_pid = getpid();
	
	int result = registerApp(app_pid, SIGSEVERE);

	if(result == 0){
		ATF_FAIL("Registration Failed");	
	}else{
		ATF_PASS();
	}
}

ATF_TEST_CASE(deregistration);
ATF_TEST_CASE_HEAD(deregistration)
{
	set_md_var("descr", "This tests deregistration of applications");
}
ATF_TEST_CASE_BODY(deregistration)
{
	int app_pid = getpid();
	
	int result = registerApp(app_pid, SIGSEVERE);

	if(result == 0){
		ATF_FAIL("Registration Failed");	
	}	
	
	//Sending registration signal again should deregister	
	result = registerApp(app_pid, SIGSEVERE);
	
	if(!result){
		ATF_PASS();
	}
	else{
		ATF_FAIL("Not Deregistered");
	}	
}

ATF_TEST_CASE(timeout);
ATF_TEST_CASE_HEAD(timeout)
{
	set_md_var("descr", "This test the proper deregistration of timedout/killed functions");
}
ATF_TEST_CASE_BODY(timeout)
{
	int child_pid = fork();
	
	int result = registerApp(child_pid, SIGSEVERE);

	if(result == 0){
		ATF_FAIL("Registration Failed");	
	}	

	kill(child_pid, SIGKILL);	

	result = registerApp(child_pid, SIGSEVERE);

	if(!result){
		ATF_PASS();
	}
	else{
		ATF_FAIL("Not Deregistered");
	}

}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, deregistration);
	ATF_ADD_TEST_CASE(tcs, registration);
	//ATF_ADD_TEST_CASE(tcs, sigstop);
	ATF_ADD_TEST_CASE(tcs, timeout);
}
