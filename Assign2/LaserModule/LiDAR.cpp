#include "LiDAR.h"

LiDAR::LiDAR(System::String^ ipaddress, int port)
{
	this->PortNumber = port;
	this->IPAddress = ipaddress;

	Client = gcnew TcpClient(IPAddress, PortNumber);
	//Client settings
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	//Declarations
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;
	
	//Initializations
	System::String^ AskID = gcnew System::String("z5224734\n");
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	NetworkStream^ Stream = Client->GetStream();

	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskID);
	// Send Authetication String
	Stream->Write(SendData, 0, SendData->Length);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	this->ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	Console::WriteLine(this->ResponseData);
}

bool LiDAR::Connect()
{
	return false;
}

void LiDAR::GetXYRangeData()
{
	//Declarations
	array<unsigned char>^ SendData = nullptr;
	array<unsigned char>^ ReadData = nullptr;
	array<System::String^>^ Fragments = nullptr;

	//Initializations
	System::String^ AskScan = gcnew System::String("sRN LMDscandata");
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	NetworkStream^ Stream = Client->GetStream();

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

	Ranges = gcnew array<double>(NumRanges);
	RangeX = gcnew array<double>(NumRanges);
	RangeY = gcnew array<double>(NumRanges);

	for (int i = 0; i < NumRanges; i++)
		{
			Ranges[i] = System::Convert::ToInt32(Fragments[26 + i], 16);
			RangeX[i] = Ranges[i] * Math::Sin(i * Resolution * Math::PI / 180.0);
			RangeY[i] = -Ranges[i] * Math::Cos(i * Resolution * Math::PI / 180.0);
		}
}

double LiDAR::GetStartAngle()
{
	return this->StartAngle;
}

double LiDAR::GetResolution()
{
	return this->Resolution;
}
