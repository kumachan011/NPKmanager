#include <iostream>
#include <Windows.h>
#include <string>
#include <filesystem>
#include <algorithm>

int main() {
	bool runProgram = true;

	while (runProgram = true) {

		char buffer[MAX_PATH];
		GetModuleFileNameA(nullptr, buffer, MAX_PATH);
		std::string dllPath = buffer;
		dllPath = dllPath.substr(0, dllPath.rfind('\\') + 1) + "MwareKeyFinder.dll";

		if (!std::filesystem::exists(dllPath)) {
			std::cout << "MwareKeyFinder.dll not found!\nPlease make sure the dll is named correctly and it is in the current programs directory, and try again.\n";
			system("pause");
			return 1;
		}

		std::cout << "MWARE KEY FINDER - kumachan011\nPlease insert the .exe path of your game here. Make sure it exists in the working directory of the game.\n\nThe program will inject a DLL into your game, prompting you to follow the instructions provided by the console.";

		std::string gamePath;
		std::getline(std::cin, gamePath);

		gamePath.erase(std::remove(gamePath.begin(), gamePath.end(), '"'), gamePath.end());
		std::string absoluteGamePath = gamePath.substr(0, gamePath.rfind('\\') + 1);

	
		if (std::filesystem::exists(gamePath)) {
			if (std::filesystem::path(gamePath).extension() == ".exe") {

				runProgram = false;

				STARTUPINFOA si{};
				PROCESS_INFORMATION pi{};
				si.cb = sizeof(si);
				BOOL openProcess = CreateProcessA(gamePath.c_str(), nullptr, nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS, nullptr, absoluteGamePath.c_str(), &si, &pi);
				if (openProcess != 0) {
					std::cout << "\nStarted game with PID " << pi.dwProcessId;
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
				else {
					std::cout << "\nCouldn't open process. Error code " << GetLastError();
					system("pause");
					return 1;
				}
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
				if (hProcess == NULL) {
					std::cout << "\nCould not open process. Error code" << GetLastError();
					system("pause");
					return 1;
				}



				LPVOID allocated_mem = VirtualAllocEx(hProcess, NULL, strlen(dllPath.c_str()) + 1, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
				if (allocated_mem == NULL) {
					std::cout << "\nCould not allocate memory. Error code " << GetLastError();
					system("pause");
					return 1;
				}
				else {
					std::cout << "\nMemory allocated at 0x" << allocated_mem << "\n";
					WriteProcessMemory(hProcess, allocated_mem, dllPath.c_str(), strlen(dllPath.c_str()) + 1, NULL);
				}
				HMODULE kernel32Base = GetModuleHandleA("kernel32.dll");
				if (kernel32Base == NULL) {
					std::cout << "\nFailed to get base of kernel32.dll in the program. Error code " << GetLastError();
					system("pause");
					return 1;
				}

				FARPROC loadLib = GetProcAddress(kernel32Base, "LoadLibraryA");
				HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLib, allocated_mem, 0, NULL);
				if (hThread == NULL) {
					std::cout << "\nFailed to create a remote thread in the program. Error code " << GetLastError();
					system("pause");
					return 1;
				}

				std::cout << "\nInjection done\n";

				WaitForSingleObject(hThread, INFINITE);
				VirtualFreeEx(hThread, allocated_mem, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				CloseHandle(hThread);
				system("pause");
				return 0;
			} else {
				std::cout << "\nFile is not an .exe!\n";
			}
		} else {
			std::cout << "\nFile doesn't exist!\n";
		}
	}

	return 0;
}
