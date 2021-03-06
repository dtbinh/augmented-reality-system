#ifndef MARS_Circular_Buffer_H_
#define MARS_Circular_Buffer_H_
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <algorithm>
using namespace std;

class MARS_Circular_Buffer{
	private:
		class Circular_BufferNode{
			friend class MARS_Circular_Buffer;
			Circular_BufferNode* next;
			char* data;
			int size;
			Circular_BufferNode(int packetlength){
				size = packetlength;
				data = new char[packetlength];
			}
			~Circular_BufferNode(){
				delete[] data;
			}
		};
		Circular_BufferNode *begin, *end;
			
	public:
		MARS_Circular_Buffer(int length, int packetlength);
		~MARS_Circular_Buffer();
		void queue (char* packet);
		void dequeue (char* toFill, int len);
		
};
#endif
