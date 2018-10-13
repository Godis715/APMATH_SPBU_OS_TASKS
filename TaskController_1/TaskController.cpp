
#define _CRT_SECURE_NO_WARNINGS
#include <thread>

#include <wchar.h>
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <TCHAR.H>
#include <iostream>

#include "CPU.h"

HANDLE GetProcessHandle(const std::string& fName) {

	const char*  fileName = fName.c_str();
	TCHAR szName[512];

	swprintf(szName, 255, L"%hs", fileName);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProc;
	bool bRes = false;
	if (Process32First(hSnap, &pe32) == TRUE)
	{
		do
		{
			if (wcscmp(pe32.szExeFile, szName) == 0)
			{
				hProc = OpenProcess(PROCESS_ALL_ACCESS,
					FALSE,
					pe32.th32ProcessID);
				return hProc;
			}
		} while (Process32Next(hSnap, &pe32) != FALSE);
	}
	return NULL;
}

bool SetProcessHightPriority(HANDLE& process) {
	return !!SetPriorityClass(process, HIGH_PRIORITY_CLASS);

}

bool SetProcessNormalPriority(HANDLE& process) {
	return !!SetPriorityClass(process, NORMAL_PRIORITY_CLASS);
}

bool toStop = false;

void ProcessListener(HANDLE& process) {
	
	using namespace std::chrono_literals;

	bool buttonPressed = false;
	while (!toStop) {
		if (getCurrentValue() > 30.0) {
			SetProcessNormalPriority(process);
			std::cout << "Normal\n";
		}
		else {
			SetProcessHightPriority(process);
			std::cout << "High\n";

		}
		buttonPressed = !buttonPressed;
		std::this_thread::sleep_for(500ms);
	}

	CloseHandle(process);
}

int main(int argc, char* argv[])
{

	init();

	HANDLE process = NULL;
	if (argc == 2) {
		std::string fName(argv[1]);
		process = GetProcessHandle(fName);

		if (process == NULL) {
			std::cout << "Process wasn't found" << std::endl;
		}
	}

	while (process == NULL) {
		std::string fName;
		std::cout << "Enter process name: ";
		std::cin >> fName;

		process = GetProcessHandle(fName);
		if (process == NULL) {
			std::cout << "Process wasn't found" << std::endl;
		}
		else {
			break;
		}
	}

	std::thread processListener(ProcessListener, std::ref(process));
	processListener.detach();

	while (true) {
		char c;
		std::cin.get(c);
		if (c == 's') {
			toStop = true;
			break;
		}
	}

	
	system("pause");
    return 0;
}

