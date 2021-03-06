#include "./utils/thread.hpp"
#include "./utils/timer.hpp"
#include "./utils/udpPort.hpp"

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <algorithm>

//OpenGL libraries
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <opencv/highgui.h>
#include <opencv/cv.h>

#include "MARS_Circular_Buffer.hpp"

using namespace std;

#ifndef MARS_Datasource_H_
#define MARS_Datasource_H_

class MARS_Datasource{
	friend class MARS_Engine;
	protected:
		char viconIP[64];
		char miscIP[64];
		int vicon_port;
		int misc_port;
		bool doneVicon;
		bool doneMisc;
		char * viconBuf;
		int vicon_bufsize;
		int misc_bufsize;

		Thread vicon_datasourceManagerThread;  //thread that runs the RAVEN datamanager
		Thread misc_datasourceManagerThread; //thread that runs the misc datamanager

		void * (*vicon_datasourceManager)(void*);
		void * (*misc_datasourceManager)(void*);

		pthread_mutex_t datamutex; //a mutex lock for the data
		//Timer timer;
		void vicon_dataLock();
		void vicon_dataUnlock();
		void misc_dataLock();
		void misc_dataUnlock();
		void getviconBuffer(char* vicondest);
		void getmiscBuffer (char* misdest);

		MARS_Circular_Buffer *misc_circ_buf;
		
	public:
		MARS_Datasource(string viconIP, int viconport_listen, int vicon_bufsize, string miscIP, int miscport_listen, int misc_bufsize);
		~MARS_Datasource();
		void * vicon_UDPDataManager();
		void * misc_UDPDataManager();
		bool run();
		void vicon_stopAndJoin();
		void misc_stopAndJoin();
		int getViconBufSize();
		int getMiscBufSize();
};
#endif
