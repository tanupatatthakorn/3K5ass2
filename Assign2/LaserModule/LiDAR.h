#pragma once

#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#using <System.dll>


using namespace System; // for console
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


ref class LiDAR
{
private:
	TcpClient^ Client;
	int PortNumber;
	System::String^ IPAddress;
	double StartAngle;
	double Resolution;
	System::String^ ResponseData;

public:
	int NumRanges;
	array<double>^ Ranges;
	array<double>^ RangeX;
	array<double>^ RangeY;
public:
	LiDAR(System::String^ ipaddress, int port);
	bool Connect();
	void GetXYRangeData();
	double GetStartAngle();
	double GetResolution();
};