#include <iostream>
#include <vector>

#include <fcntl.h> //Some C libraries are required
#include <kvm.h>
#include <libutil.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <vm/vm_param.h>
using namespace std; 

//These signals will eventually come from a .h file
//44 is a test number, the number used in deployment will require review by an architect
#define SIGTEST 44

#define SIGSEVERE 45
#define SIGMIN 46
#define SIGPAGESNEEDED 47 

#define CONVERT(v)	((int64_t)(v) * pagesize / blocksize)
#define CONVERT_BLOCKS(v) 	((int64_t)(v) * pagesize)
static struct kvm_swap swtot;
static int nswdev;
static SLIST_HEAD(slisthead, managed_application) head = SLIST_HEAD_INITIALIZER(head);
static struct slisthead *headp;

//Track all the markers we want to observe
struct memStatus
{
	bool target, min, needed, severe;
};

struct managed_application
{
	int pid, condition;
	SLIST_ENTRY(managed_application) next_application;
};
//Query the device for updates statuses. 
memStatus queryDev()
{
	memStatus status = {false,false,false,false};
	// Read the file, C++ libraries are no good for reading from a device
	int devfile = open("/dev/lowmem", O_RDWR | O_NONBLOCK);
	if(devfile >= 0){
		char buf;
		//If the transfer worked
		if (read(devfile,&buf,1)) {
		    if(buf & 0b1){
		    	status.target=true;
		    if(buf & 0b10){
		    	status.min=true;
		    if(buf & 0b100)
		    	status.needed=true;
		    if(buf & 0b1000) {
		    	status.severe=true;
		}
	}
	return status;
}

static void print_swap_stats(const char *swdevname, intmax_t nblks, intmax_t bused, intmax_t bavail, float bpercent)
{
	char usedbuf[5];
	char availbuf[5];
	int hlen, pagesize;
	long blocksize;
	const char *header;	

	pagesize = getpagesize();
	getbsize(&hlen, &blocksize);
	header = getbsize(&hlen, &blocksize);
		

//	(void)printf("%-15s %*s %8s %8s %8s\n", "Device", hlen, header, "Used", "Avail", "Capacity");
	
//	printf("%-15s %*jd ", swdevname, hlen, CONVERT(nblks));
	humanize_number(usedbuf, sizeof(usedbuf), CONVERT_BLOCKS(bused), "",
			HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
	humanize_number(availbuf, sizeof(availbuf), CONVERT_BLOCKS(bavail), "",
			HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
//	printf("%8s %8s %5.0f%%\n", usedbuf, availbuf, bpercent);
}

static void swapmode_sysctl(void)
{
	struct kvm_swap kswap;
	struct xswdev xsw;
	size_t mibsize, size;
	int mib[16], n;
	swtot.ksw_total = 0;
	swtot.ksw_used = 0;
	mibsize = sizeof mib / sizeof mib[0];
	sysctlnametomib("vm.swap_info", mib, &mibsize);
	for (n=0; ; ++n){
		mib[mibsize] = n;
		size = sizeof xsw;
		if (sysctl(mib, mibsize + 1, &xsw, &size, NULL, 0) == -1)
			break;
		kswap.ksw_used = xsw.xsw_used;
		kswap.ksw_total = xsw.xsw_nblks;
		swtot.ksw_total += kswap.ksw_total;
		swtot.ksw_used += kswap.ksw_used;
		++nswdev;	
	}
	print_swap_stats("Swap Total", swtot.ksw_total, swtot.ksw_used,
			swtot.ksw_total - swtot.ksw_used,
			(swtot.ksw_used * 100.0) / swtot.ksw_total);	
}

static void physmem_sysctl(void)
{
	int mib[2], usermem;
	size_t len;
	mib[0] = CTL_HW;
	mib[1] = HW_USERMEM;
	len = sizeof(usermem);
	sysctl(mib, 2, &usermem, &len, NULL, 0);
//	cout << "Free memory: " << usermem << endl; //change to printf
}

void monitor_application(int signal_number, siginfo_t *info, void *unused){
	struct managed_application *application = (managed_application*)malloc(sizeof(struct managed_application));
	application->pid = info->si_pid;	
	application->condition = signal_number;
	SLIST_INSERT_HEAD(&head, application, next_application);
	printf("REGISTERED\n");
}



/*
* Memory Conditions:
* 0 = Severe Low Memory
* 1 = Under Minimum Free Pages Threshold
* 2 = Not Enough Free Pages
*/

int main(int argc, char ** argv)
{
	//	daemon(0,0);
	
	if(argc != 3){
		cout << "Usage: ./daemon pid memorycondition" << endl;
		return 1;
	}
	int memoryCondition = atoi(argv[2]);

	struct sigaction sig;
	sig.sa_sigaction = monitor_application;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGSEVERE, &sig, NULL);
	sigaction(SIGMIN, &sig, NULL);
		

	SLIST_INIT(&head);
	struct managed_application *current_application = (managed_application*)malloc(sizeof(struct managed_application));
	
	for(;;){
		swapmode_sysctl();
		physmem_sysctl();
		memStatus status = queryDev();
		SLIST_FOREACH(current_application, &head, next_application){
			if(status.severe && current_application->condition == SIGSEVERE){
				int pid = current_application->pid;
				kill(pid,SIGTEST);
				printf("KILLED SEVERE: %d\n", pid);
			}
			if(status.min && current_application->condition == SIGMIN){
				int pid = current_application->pid;
				kill(pid,SIGTEST);
				printf("KILLED MIN: %d\n", pid);
			}
			if(status.needed && memoryCondition == SIGPAGESNEEDED){
				int pid = current_application->pid;
				kill(pid,SIGTEST);
				printf("KILLED PAGES NEEDED: %d\n", pid);
			}
		}
		sleep(2);
	}
	return 0;
}
