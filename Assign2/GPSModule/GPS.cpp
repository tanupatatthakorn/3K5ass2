#include "GPS.h"
// Novatel GPS BESTUTMB data structure. 112 bytes
#pragma pack(1)
struct novatel_GPS //112 bytes
{
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int Checksum;
}GPS_struct;

GPS::GPS(System::String^ ipaddress, int portNumber)
{
	this->IPAddress = ipaddress;
	this->portNumber = portNumber;
	this->Client = gcnew TcpClient(this->IPAddress, this->portNumber);
	Client->NoDelay = true;
	Client->ReceiveTimeout = 2000;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(256);	//might not need
	Stream = Client->GetStream();
}

GPS::~GPS()
{
	Stream->Close();
	Client->Close();
}

void GPS::processGPSData()
{
	Thread::Sleep(10);
	// Get incoming data
	if (Stream->DataAvailable) {
		Console::WriteLine("Data read");
		Stream->Read(ReadData, 0, ReadData->Length);
	}
	/*Print statement to observe*/
	for (int i = 0; i < ReadData->Length; i++) {
		Console::Write("{0:X2} ", ReadData[i]);
	}
	Console::WriteLine("");
	this->readFromHeader();
	// Compare CRC before setting attributes
	unsigned char* bytePtr = (unsigned char*)& GPS_struct;
	unsigned int GeneratedCRC = CalculateBlockCRC32(112 - 4, bytePtr);
	this->CRC = GPS_struct.Checksum;
	Console::WriteLine("CalcCRC: {0}, ServerCRC: {1}, Equal {2}]", GeneratedCRC, CRC, GeneratedCRC == CRC);
	/*Setting attributes*/
	if (GeneratedCRC == this->CRC) {
		this->easting = GPS_struct.Easting;
		this->northing = GPS_struct.Northing;
		this->height = GPS_struct.Height;
	}
	else {
		Console::WriteLine("GPS checksum mismatched");
	}
	Thread::Sleep(100);
}

double GPS::getNorthing()
{
	return this->northing;
}

double GPS::getEasting()
{
	return this->easting;
}

double GPS::getHeight()
{
	return this->height;
}

unsigned int GPS::getCRC()
{
	return this->CRC;
}

void GPS::readFromHeader()
{
	// Start reading from header. From there we fill GPS_struct
	// Trapping the Header
	bool hasHeader = false;
	unsigned int Header = 0;
	int i = 0;
	int Start; //Start of data
	unsigned char Data;
	do
	{
		Data = ReadData[i++];
		Header = ((Header << 8) | Data);
		if (Header == 0xaa44121c) {
			hasHeader = true;
		}
	} while (Header != 0xaa44121c && i < ReadData->Length);
	Start = i - 4;
	if (hasHeader == false) {
		return;
	}
	// Filling data
	unsigned char* BytePtr = nullptr;
	BytePtr = (unsigned char*)& GPS_struct;
	for (int i = Start; i < Start + sizeof(GPS_struct); i++)
	{
		*(BytePtr++) = ReadData[i];
	}
}

unsigned long GPS::CRC32Value(int i)
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

unsigned long GPS::CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer)
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
