#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>	// for kbhit

using namespace System; // for console
using namespace System::Threading;

int main() {
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PM* PMSMPtr = nullptr;
	int waitCount = 0;
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}
	
	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.GPS = 0;

	while (!PMSMPtr->Shutdown.Flags.GPS) {
		
		PMSMPtr->Heartbeats.Flags.GPS = 1;
		Console::WriteLine("GPS is alive");
		if (PMSMPtr->PMHeartbeats.Flags.GPS == 1) {
			Console::WriteLine("In GPS Set PMGPS LOW");
			PMSMPtr->PMHeartbeats.Flags.GPS = 0;
			waitCount = 0;
		}
		else {
			if (++waitCount > 50) {
				// we have waited too long
				PMSMPtr->Shutdown.Status = 0xFF;
			}
		}
		if (_kbhit()) break;
		Thread::Sleep(20);
	}

	Console::WriteLine("GPS Process terminated");
	Console::ReadKey();
	return 0;
}