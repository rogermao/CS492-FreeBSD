#include <iostream>
#include <vector>

#include "application.cpp"

#include <fcntl.h> //Some C libraries are required
#include <kvm.h>
#include <libutil.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/event.h>
#include <sys/sysctl.h>
#include <sys/time.h>
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
#define SIGREGISTERED 48
#define SIGDEREGISTERED 49

#define CONVERT(v)	((int64_t)(v) * pagesize / blocksize)
#define CONVERT_BLOCKS(v) 	((int64_t)(v) * pagesize)
static struct kvm_swap swtot;
static int nswdev;
struct kevent change[1];
struct kevent event[1];

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


	//(void)printf("%-15s %*s %8s %8s %8s\n", "Device", hlen, header, "Used", "Avail", "Capacity");

	//printf("%-15s %*jd ", swdevname, hlen, CONVERT(nblks));
	humanize_number(usedbuf, sizeof(usedbuf), CONVERT_BLOCKS(bused), "",
			HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
	humanize_number(availbuf, sizeof(availbuf), CONVERT_BLOCKS(bavail), "",
			HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL);
	//printf("%8s %8s %5.0f%%\n", usedbuf, availbuf, bpercent);
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


/*
* Memory Conditions:
* 0 = Severe Low Memory
* 1 = Under Minimum Free Pages Threshold
* 2 = Not Enough Free Pages
*/

int main(int argc, char ** argv)
{
	if (argc != 1){
		printf("Args: %d\n", argc);
		return -1;
	}

	pthread_t signalThread;
	pthread_create(&signalThread, 0, monitor_signals, (void*)0);
	int fd=0;
	fd = open("/dev/lowmem", O_RDWR | O_NONBLOCK);
	int kq=kqueue();
	EV_SET(&change[0],fd,EVFILT_READ, EV_ADD,0,0,0);
	for(;;){
		int flags = block(kq, change, event)

		check_queue(flags);

		check_flags(flags);

		sleep(2);

	}
	return 0;
}
