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

ATF_TEST_CASE(sigmin);
ATF_TEST_CASE_HEAD(sigmin)
{
	set_md_var("descr", "This test if sigmin signal triggered properly");
}
ATF_TEST_CASE_BODY(sigmin)
{
	received = 0;

	int app_pid = getpid();

	setupHandler(SIGTEST);
	
	int result = registerApp(app_pid, SIGMIN);

	if(result == 0){
		ATF_FAIL("Application Failed to Register");
	}

	srand(time(NULL));
	int *mem;
	int memsize=1024; 
	mem= (int*)malloc(memsize);
	
	while(queryDev().min){
		mem= (int*)realloc(mem,memsize=memsize+1024*1024*10);
		sleep(1);
	}

	check_applications(false, true, false);

	sleepFor(5);

	if(received){
		ATF_PASS();
	}else{
		ATF_FAIL("SIGMIN not received");
	}
	
}


ATF_TEST_CASE(sigpagesneeded);
ATF_TEST_CASE_HEAD(sigpagesneeded)
{
	set_md_var("descr", "This test if sigpagesneeded signal triggered properly");
}
ATF_TEST_CASE_BODY(sigpagesneeded)
{
	received = 0;

	int app_pid = getpid();

	setupHandler(SIGTEST);

	int result = registerApp(app_pid, SIGPAGESNEEDED);

	if(result == 0){
		ATF_FAIL("Application Failed to Register");
	}

	srand(time(NULL));
	int *mem;
	int memsize=1024; 
	mem= (int*)malloc(memsize);
	
	while(queryDev().needed){
		mem=(int*)realloc(mem,memsize=memsize+1024*1024*10);
		sleep(1);
	}

	check_applications(false, false, true);

	sleepFor(5);

	if(received){
		ATF_PASS();
	}else{
		ATF_FAIL("SIGPAGESNEEDED not received");
	}


}

//DISABLED: Imossible to override signal handler
ATF_TEST_CASE(sigstop);
ATF_TEST_CASE_HEAD(sigstop)
{
	set_md_var("descr", "This test suspend_applications");
}
ATF_TEST_CASE_BODY(sigstop)
{
	received = 0;

	int app_pid = getpid();

	setupHandler(SIGSTOP);

	int result = registerApp(app_pid, SIGSEVERE);

	if(result == 0){
		ATF_FAIL("Application Failed to Register");
	}

	suspend_applications();	

	sleepFor(5);

	if(received){
		ATF_PASS();
	}
	ATF_FAIL("THIS SHOULD NEVER FAIL");

}
ATF_TEST_CASE(sigcont);
ATF_TEST_CASE_HEAD(sigcont)
{
	set_md_var("descr", "This test resume_applications");
}
ATF_TEST_CASE_BODY(sigcont)
{
	received = 0;

	int app_pid = getpid();

	setupHandler(SIGCONT);

	int result = registerApp(app_pid, SIGSEVERE);

	if(result == 0){
		ATF_FAIL("Application Failed to Register");
	}

	resume_applications();	

	sleepFor(5);

	if(received){
		ATF_PASS();
	}
	ATF_FAIL("THIS SHOULD NEVER FAIL");


}

ATF_INIT_TEST_CASES(tcs)
{
	ATF_ADD_TEST_CASE(tcs, sigmin);
	ATF_ADD_TEST_CASE(tcs, sigpagesneeded);
	ATF_ADD_TEST_CASE(tcs, sigcont);
}
