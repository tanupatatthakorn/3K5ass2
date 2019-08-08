#using <System.dll>
#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#include "VehicleControl.h"

using namespace System; // for console
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main() {

	VehicleControl^ UGV = gcnew VehicleControl("192.168.1.200", 25000);

	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	SMObject XboxObj(_TEXT("XboxObj"), sizeof(Remote));

	PM* PMSMPtr = nullptr;
	Remote* XboxPtr = nullptr;
	
	PMObj.SMAccess();
	XboxObj.SMAccess();

	PMSMPtr = (PM*)PMObj.pData;
	XboxPtr = (Remote*)XboxObj.pData;

	PMSMPtr->Shutdown.Flags.Vehicle = 0;
	
	unsigned char flag = 1;
	int waitCount = 0;

	while (!PMSMPtr->Shutdown.Flags.Vehicle) {

		UGV->setControl(XboxPtr->SetSteering, XboxPtr->SetSpeed, flag);
		flag = !flag;

		PMSMPtr->Heartbeats.Flags.Vehicle = 1;
		Console::WriteLine("Vehicle is alive");
		if (PMSMPtr->PMHeartbeats.Flags.Vehicle == 1) {

			PMSMPtr->PMHeartbeats.Flags.Vehicle = 0;
			waitCount = 0;
		}
		else {
			if (++waitCount > 50) {
				// we have waited too long
				PMSMPtr->Shutdown.Status = 0xFF;
			}
		}
		
		Console::WriteLine("waitcount: " + waitCount + " Steer: " + XboxPtr->SetSteering + " Speed: " + XboxPtr->SetSpeed);
		System::Threading::Thread::Sleep(100);

	}

	Console::WriteLine("Vehicle Process terminated");
	Console::ReadKey();
	return 0;
}