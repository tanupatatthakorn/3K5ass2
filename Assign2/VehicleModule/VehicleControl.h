#pragma once
#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#include <sstream>
#include <string>
#using <System.dll>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

ref class VehicleControl
{
private:
	array<unsigned char>^ SendData;
	TcpClient^ Client;
	NetworkStream^ Stream;
	System::String^ IPaddress;
	int PortNumber;

public:
	VehicleControl();
	~VehicleControl();
	VehicleControl(System::String^ hostName, int portNumber);
	void setControl(double Steering, double Speed, unsigned char flag);
};