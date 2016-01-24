#include <iostream>
#include <vector>
#include <fcntl.h> //Some C libraries are required
#include <signal.h> //Some C libraries are required
using namespace std; 

//These signals will eventually come from a .h file
//44 is a test number, the number used in deployment will require review by an architect
#define SIGTEST 44

//Track all the markers we want to observe
struct memStatus{
	bool target, min, needed, severe;
};

//Query the device for updates statuses. 
memStatus queryDev(){
	memStatus status = {false,false,false,false};
	// Read the file, C++ libraries are no good for reading from a device
	int devfile = open("/dev/lowmem", O_RDWR | O_NONBLOCK);
	if(devfile >= 0){
		char buf;
		//If the transfer worked
		if (read(devfile,&buf,1)) {
		    if(buf & 0b1)
		    	status.target=true;
		    if(buf & 0b01)
		    	status.min=true;
		    if(buf & 0b001)
		    	status.needed=true;
		    if(buf & 0b0001) 
		    	status.severe=true;
		}
	}
	return status;
}
/*
* Memory Conditions:
* 0 = Severe Low Memory
* 1 = Under Minimum Free Pages Threshold
* 2 = Not Enough Free Pages
*
*/
int main(int argc, char ** argv){
	if(argc != 3){
		cout << "Usage: daemon pid memorycondition" << endl;
		return 1;
	}
	int memoryCondition = atoi(argv[2]);
	while(1){
		memStatus status = queryDev();
		if(status.severe && memoryCondition == 0){
			//In the future this will be pulled from a data structure
			int pid = atoi(argv[1]);
			kill(pid,SIGTEST);
			cout << "KILLED SEVERE" << endl;
			break;
		}
		if(status.min && memoryCondition == 1){
			int pid = atoi(argv[1]);
			kill(pid,SIGTEST);
			cout << "KILLED MIN" << endl;
			break;
		}
		if(status.needed && memoryCondition == 2){
			int pid = atoi(argv[1]);
			kill(pid,SIGTEST);
			cout << "KILLED NEEDED" << endl;
			break;
		}
		cout << "sleeping ..." << endl;
		sleep(2);
	}
	return 0;
}
