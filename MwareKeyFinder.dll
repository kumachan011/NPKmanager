#include <windows.h>
#include <iostream>


HANDLE hThread;
HANDLE debugThread;
HANDLE programThread;
DWORD processID;
DWORD* keyPointer;
DWORD keygenInstruction;
HMODULE Mware;
unsigned int npkVersion;
unsigned int AESKEYGENASSIST = 0xDF3A0F66;
bool endProgram = false;
bool foundAddress = false;
unsigned char key[32];


LONG WINAPI ExceptionHandler(_EXCEPTION_POINTERS* info) {
	if (info->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
		CONTEXT* contextInfo = info->ContextRecord;
		std::cout << "\nBreakpoint occured at address " << std::hex << info->ExceptionRecord->ExceptionAddress << std::dec;
		std::cout << "\nECX: " << std::hex << contextInfo->Ecx << std::dec << "\n";

		keyPointer = reinterpret_cast<DWORD*>(contextInfo->Ecx);
		std::memcpy(&key[0], keyPointer, sizeof(key));

		std::cout << "\nFound decryption key is: ";
		for (unsigned int i = 0; i < sizeof(key); i++) {
			std::cout << std::hex << (unsigned int)key[i] << " ";
		}
		std::cout << "\n\nGame NPK version: NPK" << npkVersion;
		std::cout << "\n\n\n===========================================================================\n\n" <<
			"END.\nI would recommend you copy the decryption key for future use.\n\n";
		system("pause");
		return TRUE;
	}
}

LONG WINAPI mainThread(HMODULE hModule) {
	AllocConsole();

	processID = GetCurrentProcessId();
	HANDLE nitroProgram = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processID);
	AddVectoredExceptionHandler(1, ExceptionHandler);
	DWORD oldProtect = NULL;

	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	printf("NITROPLUS DECRYPTION KEY FINDER\n");

	BYTE* address;
	//FARPROC npkVerFinder = GetProcAddress(Mware, "ZSTDMT_endStream");
	if (!GetProcAddress(Mware, "ZSTDMT_endStream")) {
		npkVersion = 2;
		address = reinterpret_cast<BYTE*>(0x10000000);
	}
	else {
		npkVersion = 3;
		address = reinterpret_cast<BYTE*>(GetProcAddress(Mware, "ZSTDMT_endStream"));
	}
	BYTE* breakpoint = reinterpret_cast<BYTE*>(0xCC);
	MEMORY_BASIC_INFORMATION memInfo;

	std::cout << "\nSearching AESKEYGENASSIST instruction, this might take a few seconds...\nDO NOT PRESS THE \"Start Game\" BUTTON IN THE MEANTIME!\nPlease make sure this window is also active.";

	do {
		address += 0x01;
		SIZE_T memQuery = VirtualQuery(address, &memInfo, sizeof(memInfo));
		if (memQuery == 0) {
			std::cout << "\nCouldn't query memory. Error code " << GetLastError();
			return FALSE;
		}
		//std::cout << memInfo.Protect << "\n";
		unsigned int tempMem;
		std::memcpy(&tempMem, (LPCVOID)address, 4);
		if (memInfo.Protect != PAGE_NOACCESS){
			if (tempMem == AESKEYGENASSIST)
			{
				foundAddress = true;
				std::cout << "\n\nFound instruction address at 0x" <<  std::uppercase << std::hex << (unsigned int)address << std::dec << "\n";
				//address = reinterpret_cast<DWORD*>(addressChar);
			}
		}
	} while (foundAddress == false);

	BOOL protect = VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (protect == FALSE) {
		std::cout << "\nCouldn't change protection of assembly instruction in memory. Error code: " << GetLastError;
	}
	*address = 0xCC;
	std::cout << "\nAdded breakpoint at AESKEYGENASSIST instruction with pointer toward the decryption key.\nPlease press the start game button.\n\n=========================================================================== \n \n";

	while (!GetAsyncKeyState(VK_END)) {
		Sleep(100);
	}

	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return TRUE;
}


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved) 
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		MessageBoxA(NULL, "Attached to program, starting KEY FINDER!", "START", MB_ICONINFORMATION);

		Mware = GetModuleHandleA("Mware.dll");
		if (Mware == NULL) {
			MessageBoxA(NULL, "Mware.dll not found. Selected game might not be a valid Nitroplus game?", "ERROR", MB_ICONERROR);
			return FALSE;
		}
		else {
			hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)mainThread, hModule, 0, nullptr);
			if (hThread == NULL) {
				MessageBoxA(NULL, "Could not create thread.", "ERROR", MB_ICONERROR);
				return FALSE;
			}
		}

		CloseHandle(hThread);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		MessageBoxA(NULL, "Dettached from program, closing KEY FINDER.", "END", MB_ICONINFORMATION);
		break;
	}

	return TRUE;
}
