#include "MARS_Datasource.hpp"


void * vicon_UDPDataManagerStarter(void * vicon_data) {
	printf ("Starting Vicon UDPDataManagerStarter...\n");
	MARS_Datasource* udpsrc = (MARS_Datasource*) vicon_data;
	udpsrc->vicon_UDPDataManager();
}

void * misc_UDPDataManagerStarter(void * misc_data) {
	printf ("Starting Misc UDPDataManagerStarter...\n");
	MARS_Datasource* udpsrc = (MARS_Datasource*) misc_data;
	udpsrc->misc_UDPDataManager();
}

void MARS_Datasource::vicon_dataLock(){
	pthread_mutex_lock(&datamutex);
}

void MARS_Datasource::vicon_dataUnlock(){
	pthread_mutex_unlock(&datamutex);
	//pthread_yield(); //might need to yield here because this lock is probably highly contended,
			// and we don't want the unlocking thread to immediately re-lock and starve the other.
			//invoking the scheduler is slow, yes, but for most purposes this should be fine.
			//if it's not OK, change this to use spinlocks or something
			//also pray to the gods of threading - a sacrifice of a young goat often helps.
}

void MARS_Datasource::misc_dataLock(){
	pthread_mutex_lock(&datamutex);
}

void MARS_Datasource::misc_dataUnlock(){
	pthread_mutex_unlock(&datamutex);
	//pthread_yield(); //might need to yield here because this lock is probably highly contended,
			// and we don't want the unlocking thread to immediately re-lock and starve the other.
			//invoking the scheduler is slow, yes, but for most purposes this should be fine.
			//if it's not OK, change this to use spinlocks or something
			//also pray to the gods of threading - a sacrifice of a young goat often helps.
}

void MARS_Datasource::getviconBuffer(char* dest){
	this->vicon_dataLock();
	strcpy(dest, this->viconBuf);
	this->vicon_dataUnlock();
}

void MARS_Datasource::getmiscBuffer(char* dest){
	//get misc buf from linked list and delete that entry.
	misc_circ_buf->dequeue(dest, misc_bufsize);
	//this->dataLock();
	//strcpy(dest, this->miscBuf);
	//this->dataUnlock();
}

MARS_Datasource::MARS_Datasource(string viconIP, int viconport_listen, int vicon_bufsize, string miscIP, int miscport_listen, int misc_bufsize){
	strncpy(this->viconIP, viconIP.c_str(), 64);
	strncpy(this->miscIP, miscIP.c_str(), 64);

	this->vicon_port = viconport_listen;
	this->misc_port = miscport_listen;
	pthread_mutex_init(&datamutex, NULL);

	doneVicon = false;
	doneMisc = false;

	this->viconBuf = new char[vicon_bufsize];

	this->vicon_bufsize = vicon_bufsize;
	this->misc_bufsize = misc_bufsize;	

	this->vicon_datasourceManager = vicon_UDPDataManagerStarter;
	this->misc_datasourceManager = misc_UDPDataManagerStarter;
	
	misc_circ_buf = new MARS_Circular_Buffer (1000, misc_bufsize);

}

MARS_Datasource::~MARS_Datasource(){
	delete[] this->viconBuf;
	delete misc_circ_buf;
}

void * MARS_Datasource::vicon_UDPDataManager(){
	printf("Starting UDP Data Manager...\n");
	printf("On IP: %s\n", this->viconIP);
	printf("Listening to port: %d\n", this->vicon_port);
   	//## Setup UDP
    	UDPPort udp;
    	bool broadcast = false;
    	udp.udpInit(broadcast,this->viconIP,this->vicon_port);
    	char tempbuf[2048];
    	// Main loop
    	while( this->doneVicon == false )
    	{
    		strcpy(tempbuf, udp.udpReceive());
		//printf("%s\n", tempbuf);
		// Grab vicon packet
        	this->vicon_dataLock();
        	bzero(viconBuf, 2048);
        	strcpy(viconBuf, tempbuf);
        	//printf("Vicon Buffer: %s\n", viconBuf); fflush(stdout);
        	this->vicon_dataUnlock();
	}
    	// end while() for UDP
	//check whether datasource contains miscData or viconData	 
    	udp.udpClose();
    	//## Exit cleanly
    	sched_yield();
    	usleep(1000);
	printf("Exiting UDP Data Manager...\n");
    	return(0);
}

void * MARS_Datasource::misc_UDPDataManager(){
	printf("Starting Misc UDP Data Manager...\n");
	printf("On Misc IP: %s\n", this->miscIP);
	printf("Listening to Misc port: %d\n", this->misc_port);
   	//## Setup UDP
    	UDPPort udp;
    	bool broadcast = false;
    	udp.udpInit(broadcast,this->miscIP,this->misc_port);
    	char tempbuf[misc_bufsize];
    	// Main loop
    	while(this->doneMisc == false)
    	{
    		strcpy(tempbuf, udp.udpReceive());
		//printf("%s\n", tempbuf);
		float timestamp;
		this->misc_dataLock();
		misc_circ_buf->queue (tempbuf);
		printf("Misc Buffer: %s\n", tempbuf); fflush(stdout);
		this->misc_dataUnlock();
		//Insert buffer into circular linked list


    	}// end while() for UDP
	//check whether datasource contains miscData or viconData	 
    	udp.udpClose();
    	//## Exit cleanly
    	sched_yield();
    	usleep(1000);
	printf("Exiting Misc UDP Data Manager...\n");
    	return(0);
}

bool MARS_Datasource::run(){		
	if (!vicon_datasourceManagerThread.start(vicon_datasourceManager,this)){
		printf("ERROR: Could not start the Vicon datasource thread in MARS_Datasource::Viconrun().\n");
		printf("NOTE: Please make sure you have administrative privileges when running this.\n");
		printf("\tto allow the datasource thread to start.\n");
		return false;
	}
	if (!misc_datasourceManagerThread.start(misc_datasourceManager,this)){
		printf("ERROR: Could not start the misc datasource thread in MARS_Datasource::Miscrun().\n");
		printf("NOTE: Please make sure you have administrative privileges when running this.\n");
		printf("\tto allow the datasource thread to start.\n");
		return false;
	}
	return true; 
}

void MARS_Datasource::vicon_stopAndJoin(){
	this->doneVicon = true;
	if (!this->vicon_datasourceManagerThread.join())
		printf("ERROR: Vicon Datasource Thread did not join correctly in MARS_Datasource::vicon_stopAndJoin()!\n");
}

void MARS_Datasource::misc_stopAndJoin(){
	this->doneMisc = true;
	if (!this->misc_datasourceManagerThread.join())
		printf("ERROR: Misc Datasource Thread did not join correctly in MARS_Datasource::misc_stopAndJoin()!\n");
}


int MARS_Datasource::getViconBufSize(){
	return this->vicon_bufsize;
}

int MARS_Datasource::getMiscBufSize(){
	return this->misc_bufsize;
}
