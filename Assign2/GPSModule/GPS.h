#pragma once
#using <System.dll>
#include <conio.h>
#include <SMObject.h>
#include <SMStructs.h>

#define CRC32_POLYNOMIAL 0xEDB88320L
#define NO_ERROR 0
#define PORT_OPEN_ERROR 2
#define READ_TIMEOUT 3
#define NO_HEADER 4
#define CRC_ERROR 5

using namespace System; // for console
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

ref class GPS {
private:
	/*Private attributes*/
	TcpClient^ Client;
	System::String^ IPAddress;
	NetworkStream^ Stream;
	int portNumber;
	double northing;
	double easting;
	double height;
	unsigned int CRC;
	array<unsigned char>^ SendData;
	array<unsigned char>^ ReadData;
public:
	GPS(System::String^ ipaddress, int portNumber);
	~GPS();
	void processGPSData();
	double getNorthing();
	double getEasting();
	double getHeight();
	unsigned int getCRC();
	void readFromHeader();
	unsigned long CRC32Value(int i);
	unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);
};
