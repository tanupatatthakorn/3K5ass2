#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>	// for kbhit
#include <iostream>
#include <TlHelp32.h>

#define UNIT_PROCESS 5

using namespace System; // for console
using namespace System::Threading;

// Start up sequence
TCHAR* Units[10] = //
{
	TEXT("LASERModule.exe"),
	TEXT("DisplayModule.exe"),
	TEXT("GPSModule.exe"),
	TEXT("XBoxModule.exe"),
	TEXT("VehicleModule.exe"),
	

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
	SMObject GPSObj(_TEXT("GPSObj"), sizeof(GPS));
	SMObject XboxObj(_TEXT("XboxObj"), sizeof(Remote));
	
	PM* PMSMPtr = nullptr;
	Laser* LaserPtr = nullptr;
	GPS* GPSPtr = nullptr;
	Remote* XboxPtr = nullptr;

	//Create/Access SM
	//PM
	PMObj.SMCreate();
	PMObj.SMAccess();
	//Laser
	LaserObj.SMCreate();
	LaserObj.SMAccess();
	//GPS
	GPSObj.SMCreate();
	GPSObj.SMAccess();
	//Xbox
	XboxObj.SMCreate();
	XboxObj.SMAccess();
	
	//Access to pointers
	PMSMPtr = (PM*)PMObj.pData;
	LaserPtr = (Laser*)LaserObj.pData;
	XboxPtr = (Remote*)XboxObj.pData;

	PMSMPtr->Shutdown.Flags.PM = 0;
	PMSMPtr->Heartbeats.Status = 0xFF;

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
	//main loop
	while (!PMSMPtr->Shutdown.Flags.PM) {

		// GPS must have set this to HIGH
		PMSMPtr->PMHeartbeats.Status = 0xFF;

		Thread::Sleep(300);

		//--------GPS Heartbeats--------
		if (PMSMPtr->Heartbeats.Flags.GPS == 1) {
			PMSMPtr->Heartbeats.Flags.GPS = 0;
		}
		else {
			// restart
			if (!IsProcessRunning(Units[2]))
			{
				ZeroMemory(&s[2], sizeof(s[2]));
				s[2].cb = sizeof(s[2]);
				ZeroMemory(&p[2], sizeof(p[2]));
				// Start the child processes.

				if (!CreateProcess(NULL,   // No module name (use command line)
					Units[2],        // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					CREATE_NEW_CONSOLE,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory
					&s[2],            // Pointer to STARTUPINFO structure
					&p[2])           // Pointer to PROCESS_INFORMATION structure
					)
				{
					printf("%s failed (%d).\n", Units[2], GetLastError());
					_getch();
					return -1;
				}
			}
			//std::cout << "Started: " << Units[2] << std::endl;
		}


		//--------Laser Heartbeats----------
		PMSMPtr->PMHeartbeats.Flags.Laser = 1;
		if (PMSMPtr->Heartbeats.Flags.Laser == 1) {
			
			PMSMPtr->Heartbeats.Flags.Laser = 0;
		}
		else {
			Console::WriteLine("Laser failed in PM");
			PMSMPtr->Shutdown.Status = 0xFF;
		}


		//--------Xbox Heartbeats----------
		PMSMPtr->PMHeartbeats.Flags.Xbox = 1;
		if (PMSMPtr->Heartbeats.Flags.Xbox == 1) {

			PMSMPtr->Heartbeats.Flags.Xbox = 0;
		}
		else {
			Console::WriteLine("Xbox failed in PM");
			PMSMPtr->Shutdown.Status = 0xFF;
		}

		//--------Vehicle Heartbeats--------
		PMSMPtr->PMHeartbeats.Flags.Vehicle = 1;
		if (PMSMPtr->Heartbeats.Flags.Vehicle == 1) {

			PMSMPtr->Heartbeats.Flags.Vehicle = 0;
		}
		else {
			Console::WriteLine("Vehicle failed in PM");
			PMSMPtr->Shutdown.Status = 0xFF;
		}



		//if (_kbhit()) {
		//	PMSMPtr->Shutdown.Status = 0xFF;
		//	bool ShutdownAll = false;
		//	while (!ShutdownAll) {
		//		for (int i = 0; i < UNIT_PROCESS; i++) {
		//			if (IsProcessRunning(Units[i])) {
		//				ShutdownAll = false;
		//				break;
		//			}
		//			else {
		//				ShutdownAll = true;
		//			}

		//		}
		//	}
		//}

		/*if (_kbhit()) {
			PMSMPtr->Shutdown.Status = 0xFE;
			bool allShutdown = false;
			while (!allShutdown) {
				if (PMSMPtr->Shutdown.Status == 0xFE) {
					PMSMPtr->Shutdown.Flags.PM = 1;
					allShutdown = true;
				}
			}
		}*/

		if (_kbhit())
			PMSMPtr->Shutdown.Flags.PM =1;

	Thread::Sleep(10);
	}

	
	system("taskkill /F /T /IM LaserModule.exe");
	system("taskkill /F /T /IM GPSModule.exe");
	system("taskkill /F /T /IM VehicleModule.exe");
	system("taskkill /F /T /IM DisplayModule.exe");
	system("taskkill /F /T /IM XBoxModule.exe");

	Console::WriteLine("Process manager terminated");
	Console::ReadKey();
	return 0;
}