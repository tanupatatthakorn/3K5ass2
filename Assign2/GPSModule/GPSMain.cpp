#using <System.dll>
#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>	// for kbhit
#define CRC32_POLYNOMIAL		0xEDB88320L

#pragma pack(1)

using namespace System; // for console
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


struct GData {
	unsigned int Header;
	unsigned char discard1[40];
	double northing;
	double easting;
	double hgt;
	unsigned char discard2[40];
	unsigned int CRC;
};



unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer); /* Data block */

int main() {
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	SMObject GPSObj(_TEXT("GPSObj"), sizeof(GPS));
	GData GD;

	PM* PMSMPtr = nullptr;
	GPS* GPSPtr = nullptr;
	int waitCount = 0;
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}
	GPSObj.SMAccess();
	if (GPSObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}

	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.GPS = 0;
	GPSPtr = (GPS*)GPSObj.pData;
	// GPS uses PORT number 24000
	int PortNumber = 24000;
	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", 24000);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 2000;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;
	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(256);

	// Get the network streab object associated with client so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	while (!PMSMPtr->Shutdown.Flags.GPS) {

		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		Thread::Sleep(10);

		if (Stream->DataAvailable) {
			Console::WriteLine("Data available");
		}

		Stream->Read(ReadData, 0, ReadData->Length);

		//Check read data for GPS
		//for (int i = 0; i < ReadData->Length; i++) {
		//	Console::Write("{0:X2} ", ReadData[i]);
		//}

		unsigned char* BytePtr = (unsigned char*)& GD;

		unsigned int Header = 0;
		int i = 0;
		unsigned char Data = 0;
		bool headerFound = false;
		int Start; //Start of data
		do
		{
			Data = ReadData[i++];
			Header = ((Header << 8) | Data);
			if (Header == 0xaa44121c)

				headerFound = true;
		} while (Header != 0xaa44121c && i < ReadData->Length);
		Start = i - 4;
		if (!headerFound) continue;

		for (int i = Start; i < Start + sizeof(GD); i++) {
			*(BytePtr++) = ReadData[i];
		}

		unsigned char* ptr = (unsigned char*)& GD;
		unsigned int GeneratedCRC = CalculateBlockCRC32(108, ptr);

		if (GeneratedCRC == GD.CRC) { //check if prints correctly
			//added to SM
			GPSPtr->Northing = GD.northing;
			GPSPtr->Easting = GD.easting;
			GPSPtr->Height = GD.hgt;
			Console::WriteLine("N: " + GPSPtr->Northing + " E: " + GPSPtr->Easting + " H: " + GPSPtr->Height + " CalCRC: " + GeneratedCRC + " CRC: " + GD.CRC);
			
		}

		PMSMPtr->Heartbeats.Flags.GPS = 1;
		if (PMSMPtr->PMHeartbeats.Flags.GPS == 1) {

			PMSMPtr->PMHeartbeats.Flags.GPS = 0;
			waitCount = 0;
		}
		else {
			if (++waitCount > 50) {
				// we have waited too long
				PMSMPtr->Shutdown.Status = 0xFF;
			}
		}
		
		Console::WriteLine("GPS is alive" + "waitcount: " + waitCount);
		Thread::Sleep(20);
	}

	Console::WriteLine("GPS Process terminated");
	//Console::ReadKey();
	return 0;
}

//CRC functions
unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}

//Note: Data Block ucBuffer should contain all data bytes of the full data record except the checksum bytes.