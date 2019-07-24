#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>	// for kbhit
#include <iostream>
#include <TlHelp32.h>

using namespace System; // for console
using namespace System::Threading;

// Start up sequence
TCHAR* Units[10] = //
{
	TEXT("LASERModule.exe"),
	TEXT("GPSModule.exe"),
	TEXT("XBoxModule.exe"),
	TEXT("VehicleModule.exe"),
	TEXT("OpenGLModule.exe"),

};

// Module execution based variable declarations
STARTUPINFO s[10];
PROCESS_INFORMATION p[10];

//Is provess running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


int main() {
	SMObject PMObj(_TEXT("PMObj"), sizeof(PM));
	PM* PMSMPtr = nullptr;

	PMObj.SMCreate();
	if (PMObj.SMCreateError) {
		Console::WriteLine("Shared memory creation failed");
		return -1;
	}

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		Console::WriteLine("Shared memory access failed");
		return -2;
	}

	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.PM = 0;
	PMSMPtr->Heartbeats.Status = 0xFF;

	// Starting the processes
	for (int i = 0; i < 2; i++)
	{
		// Check if each process is running
		if (!IsProcessRunning(Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));
			// Start the child processes.

			if (!CreateProcess(NULL,   // No module name (use command line)
				Units[i],        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory
				&s[i],            // Pointer to STARTUPINFO structure
				&p[i])           // Pointer to PROCESS_INFORMATION structure
				)
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
				return -1;
			}
		}
		std::cout << "Started: " << Units[i] << std::endl;
		Sleep(1000);
	}

	while (!PMSMPtr->Shutdown.Flags.PM) {

		Thread::Sleep(200);
		// GPS must have set this to HIGH
		PMSMPtr->PMHeartbeats.Status = 0xFF;
		if (PMSMPtr->Heartbeats.Flags.GPS == 1) {
			PMSMPtr->Heartbeats.Flags.GPS = 0;
		}
		else {
			// if GPS is critical we shutdown all
			PMSMPtr->Shutdown.Status = 0xFF;
		}

		//// Laser must have set this to HIGH
		//PMSMPtr->PMHeartbeats.Flags.Laser = 1;
		//if (PMSMPtr->Heartbeats.Flags.Laser == 1) {
		//	//Console::WriteLine("In PM Set GPS HIGH");
		//	Console::WriteLine(PMSMPtr->Heartbeats.Flags.Laser);
		//	PMSMPtr->Heartbeats.Flags.Laser = 0;
		//	Console::WriteLine(PMSMPtr->Heartbeats.Flags.Laser);
		//	PMSMPtr->PMHeartbeats.Flags.Laser = 1;
		//}
		//else {
		//	// if GPS is critical we shutdown all
		//	//PMSMPtr->Shutdown.Status = 0xFF;
		//}

		if (_kbhit()) {
			PMSMPtr->Shutdown.Status = 0xFE;
			Thread::Sleep(1000);
			break;
		}

	
		Thread::Sleep(10);
		Console::WriteLine("GPS Hearthbeats: " + PMSMPtr->Heartbeats.Flags.GPS);
	}
	Console::WriteLine("Process manager terminated");
	Console::ReadKey();
	return 0;
}