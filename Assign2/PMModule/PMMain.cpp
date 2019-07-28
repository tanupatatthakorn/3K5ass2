#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>	// for kbhit
#include <iostream>
#include <TlHelp32.h>

#define UNIT_PROCESS 2

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
	SMObject LaserObj(_TEXT("LaserObj"), sizeof(Laser));
	PM* PMSMPtr = nullptr;
	Laser* LaserPtr = nullptr;

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

	LaserObj.SMCreate();
	LaserObj.SMAccess();

	PMSMPtr = (PM*)PMObj.pData;
	PMSMPtr->Shutdown.Flags.PM = 0;
	PMSMPtr->Heartbeats.Status = 0xFF;

	LaserPtr = (Laser*)LaserObj.pData;

	// Starting the processes
	for (int i = 0; i < UNIT_PROCESS; i++)
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
		Sleep(200);
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

		PMSMPtr->PMHeartbeats.Flags.Laser = 1;
		if (PMSMPtr->Heartbeats.Flags.Laser == 1) {
			
			PMSMPtr->Heartbeats.Flags.Laser = 0;
		}
		else {
			// if GPS is critical we shutdown all
			PMSMPtr->Shutdown.Status = 0xFF;
		}


		

		/*if (_kbhit()) {
			PMSMPtr->Shutdown.Status = 0xFF;
			bool ShutdownAll = false;
			while (!ShutdownAll) {
				for (int i = 0; i < UNIT_PROCESS; i++) {
					if (IsProcessRunning(Units[i])) {
						ShutdownAll = false;
						break;
					}
					else {
						ShutdownAll = true;
					}

				}
			}
		}*/

		if (_kbhit()) {
			PMSMPtr->Shutdown.Status = 0xFE;
			bool allShutdown = false;
			while (!allShutdown) {
				if (PMSMPtr->Shutdown.Status == 0xFE) {
					PMSMPtr->Shutdown.Flags.PM = 1;
					allShutdown = true;
				}
			}
		}

		Thread::Sleep(10);
		Console::WriteLine("GPS Hearthbeats: {0}", PMSMPtr->Heartbeats.Flags.GPS);
		Console::WriteLine("Laser Heartbeats: " + PMSMPtr->Heartbeats.Flags.Laser);
	}
	Console::ReadKey();
	Console::WriteLine("Process manager terminated");
	Console::ReadKey();
	return 0;
}