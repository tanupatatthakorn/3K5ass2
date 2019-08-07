#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#using <System.dll>

#include "XBoxController.h"
#include "XInputWrapper.h"


using namespace System; // for console
using namespace System::Threading;
using namespace System::Text;


int main() {

	XInputWrapper xinput;
	GamePad::XBoxController player(&xinput, 0);

	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	SMObject XboxObj(_TEXT("XboxObj"), sizeof(Remote));

	PM* PMSMPtr = nullptr;
	Remote* XboxPtr = nullptr;

	PMObj.SMCreate();
	XboxObj.SMCreate();
	PMObj.SMAccess();
	XboxObj.SMAccess();

	PMSMPtr = (PM*)PMObj.pData;
	XboxPtr = (Remote*)XboxObj.pData;

	PMSMPtr->Shutdown.Flags.Xbox = 0;

	int waitCount;
	player.SetDeadzone(5000);

	while (!PMSMPtr->Shutdown.Flags.Xbox) {

		double steer = (player.RightThumbLocation().GetX() - (-32767.0)) / (32767.0 - (-32767.0)) * (40.0 - (-40.0)) + (-40.0);
		double speed = (player.LeftThumbLocation().GetY() - (-32767.0)) / (32767.0 - (-32767.0)) * (1.0- (-1.0)) + (-1.0);

		XboxPtr->SetSteering = steer;
		XboxPtr->SetSpeed = speed;

		

		if (player.PressedA()) {
			PMSMPtr->Shutdown.Status = 0xFF;
		}

		if (player.IsConnected()) {
			XboxPtr->SetSteering = 0;
			XboxPtr->SetSpeed = 0;
			Console::WriteLine("Disconnected");
		}

		Console::WriteLine("Steer: " + XboxPtr->SetSteering + " " + "Speed: " + XboxPtr->SetSpeed);

		//PMSMPtr->Heartbeats.Flags.Xbox = 1;
		//if (PMSMPtr->PMHeartbeats.Flags.Xbox == 1) {

		//	PMSMPtr->PMHeartbeats.Flags.Xbox = 0;
		//	waitCount = 0;
		//}
		//else {
		//	if (++waitCount > 50) {
		//		// we have waited too long
		//		Console::WriteLine("we have waited too long");
		//		PMSMPtr->Shutdown.Status = 0xFF;
		//	}
		//}

		System::Threading::Thread::Sleep(100);

	}

	Console::WriteLine("Process terminated");
	return 0;
}