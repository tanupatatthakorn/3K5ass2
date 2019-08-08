#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#using <System.dll>

#include "XBoxController.h"
#include "XInputWrapper.h"

#define MAX_VALUE 32767.0
#define MIN_VALUE -32767.0

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

	PMObj.SMAccess();
	XboxObj.SMAccess();

	PMSMPtr = (PM*)PMObj.pData;
	XboxPtr = (Remote*)XboxObj.pData;

	PMSMPtr->Shutdown.Flags.Xbox = 0;

	int waitCount = 0;
	player.SetDeadzone(9600);

	while (!PMSMPtr->Shutdown.Flags.Xbox) {

		if (player.IsConnected() == ERROR_SUCCESS) {
			double steer = (player.RightThumbLocation().GetX() - MIN_VALUE) / (MAX_VALUE - MIN_VALUE) * (40.0 - (-40.0)) + (-40.0);
			double speed = (player.LeftThumbLocation().GetY() - MIN_VALUE) / (MAX_VALUE - MIN_VALUE) * (1.0 - (-1.0)) + (-1.0);

			XboxPtr->SetSteering = steer;
			XboxPtr->SetSpeed = speed;
		}
		else {
			XboxPtr->SetSteering = 0;
			XboxPtr->SetSpeed = 0;
			Console::WriteLine("Disconnected");
		}
		
		Console::WriteLine("Steer: " + XboxPtr->SetSteering + " " + "Speed: " + XboxPtr->SetSpeed);

		if (player.PressedA()) {
			PMSMPtr->Shutdown.Status = 0xFF;
		}


		PMSMPtr->Heartbeats.Flags.Xbox = 1;
		if (PMSMPtr->PMHeartbeats.Flags.Xbox == 1) {

			PMSMPtr->PMHeartbeats.Flags.Xbox = 0;
			waitCount = 0;
		}
		else {
			if (++waitCount > 50) {
				// we have waited too long
				Console::WriteLine("we have waited too long");
				PMSMPtr->Shutdown.Status = 0xFF;
			}
		}

		Console::WriteLine("waitCount: " + waitCount);
		System::Threading::Thread::Sleep(100);

	}

	Console::WriteLine("Process terminated");
	//Console::ReadKey();
	return 0;
}