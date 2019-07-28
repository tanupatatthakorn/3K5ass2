//#include <SMObject.h>
//#include <SMStructs.h>
//#include <conio.h>
//#using <System.dll>

#include "LiDAR.h"

using namespace System; // for console
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {
	//SMObjects
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	SMObject LaserObj(_TEXT("LaserObj"), sizeof(Laser));
	//Instantiate LiDAR Obj
	LiDAR^ MyLaser = gcnew LiDAR("192.168.1.200", 23000);
	PM* PMSMPtr = nullptr;
	Laser* LaserPtr = nullptr;

	PMObj.SMAccess();
	LaserObj.SMAccess();

	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}
	if (LaserObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}

	//Instantiate pointers
	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.Laser = 0;
	LaserPtr = (Laser*)LaserObj.pData;
	

	//initiate wait count
	int waitCount = 0;
	

	

	/*
	// LMS151 port number must be 23000
	int PortNumber = 23000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;
	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	System::String^ AskScan = gcnew System::String("sRN LMDscandata");
	System::String^ AskID = gcnew System::String("z5224734\n");
	// String to store received data for display
	System::String^ ResponseData;

	//create data
	double StartAngle;
	double Resolution;
	int NumRanges;
	array<System::String^>^ Fragments = nullptr;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);


	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskID);
	// Send Authetication String
	Stream->Write(SendData, 0, SendData->Length);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	Console::WriteLine(ResponseData);
	*/

	while (!PMSMPtr->Shutdown.Flags.Laser) {
		/*
		SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

		// Write command asking for data
		Stream->WriteByte(0x02);
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03);
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		// Print the received string on the screen
		//Console::WriteLine(ResponseData);

		//point to array of pointers
		Fragments = ResponseData->Split(' ');

		StartAngle = System::Convert::ToInt32(Fragments[23], 16);
		Resolution = System::Convert::ToInt32(Fragments[24], 16) / 10000.0;
		NumRanges = System::Convert::ToInt32(Fragments[25], 16);

		array<double>^ Range = gcnew array<double>(NumRanges);
		array<double>^ RangeX = gcnew array<double>(NumRanges);
		array<double>^ RangeY = gcnew array<double>(NumRanges);

		//for (int i = 0; i < NumRanges; i++)
		//{
		//	Range[i] = System::Convert::ToInt32(Fragments[26 + i], 16);
		//	RangeX[i] = Range[i] * Math::Sin(i * Resolution * Math::PI / 180.0);
		//	RangeY[i] = -Range[i] * Math::Cos(i * Resolution * Math::PI / 180.0);
		//	//Console::WriteLine("Range " + Range[i] + " (X,Y) " + RangeX[i] + " " + RangeY[i] + " " + NumRanges);
		//}
		if (NumRanges != 361) continue;
		Console::WriteLine("NumePoints: " + NumRanges + " Start Angle: " + StartAngle + " Resolution: " + Resolution);
		*/

		int num = LaserPtr->NumRanges;

		for (int i = 0; i < num; i++) {
			LaserPtr->XRange[i] = MyLaser->RangeX[i];
			LaserPtr->YRange[i] = MyLaser->RangeY[i];
		}

		Console::WriteLine("Angle: " + MyLaser->GetStartAngle() + " Reso: " + MyLaser->GetResolution());

		PMSMPtr->Heartbeats.Flags.Laser = 1;
		if (PMSMPtr->PMHeartbeats.Flags.Laser == 1) {
			
			PMSMPtr->PMHeartbeats.Flags.Laser = 0;
			waitCount = 0;
		}
		else {
			if (++waitCount > 50) {
				// we have waited too long
				PMSMPtr->Shutdown.Status = 0xFF;
			}
			
		}
		//if (_kbhit()) break;
		Thread::Sleep(20);
			
	}


	Console::WriteLine("Laser Process terminated");
	//Console::ReadKey();
	return 0;
}