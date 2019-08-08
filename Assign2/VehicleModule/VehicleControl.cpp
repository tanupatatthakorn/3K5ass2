#include "VehicleControl.h"

VehicleControl::VehicleControl()
{
	
}

VehicleControl::~VehicleControl()
{
	Stream->Close();
	Client->Close();
}

VehicleControl::VehicleControl(System::String^ ipaddress, int portNumber)
{
	IPaddress = ipaddress;
	PortNumber = portNumber;
	//Create Client
	Client = gcnew TcpClient(IPaddress, PortNumber);
	//Config
	Client->NoDelay = true;
	Client->ReceiveTimeout = 2000;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	array<unsigned char>^ ReadData = nullptr;

	System::String^ AskID = gcnew System::String("z5224734\n");
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	this->Stream = Client->GetStream();

	SendData = System::Text::Encoding::ASCII->GetBytes(AskID);

	Stream->Write(SendData, 0, SendData->Length);
	System::Threading::Thread::Sleep(10);
	Stream->Read(ReadData, 0, ReadData->Length);

	System::String^ ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	Console::WriteLine(ResponseData);
}

void VehicleControl::setControl(double Steering, double Speed, unsigned char flag)
{
	auto SteerStr = std::to_string(-Steering);
	auto SpeedStr = std::to_string(Speed);
	auto flagStr = std::to_string(flag);
	auto SendString = "# " + SteerStr + " " + SpeedStr + " " + flagStr + " #";

	Console::WriteLine(SendString.c_str());
	System::String^ VControl = gcnew System::String(SendString.c_str());
	SendData = System::Text::Encoding::ASCII->GetBytes(VControl);

	Stream->WriteByte(0x2);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x3);

	System::Threading::Thread::Sleep(10);
}
