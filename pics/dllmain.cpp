﻿/*
The detour frame is adapted from https://guidedhacking.com/threads/c-detour-hooking-function-tutorial-for-game-hacking.7930
*/
#include "stdafx.h"
#include <Windows.h>
#include<Psapi.h>
#include<string>

#pragma warning(disable:4996)


DWORD championCode; //championID

bool Hook(void * toHook, void * ourFunct, const int len) {
	if (len < 5) {
		return false;
	}

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection); //Change to PAGE_EXECUTE_READWRITE

	memset(toHook, 0x90, len); //Set to nop

	DWORD relativeAddress = ((DWORD)ourFunct - (DWORD)toHook) - 5;

	*(BYTE*)toHook = 0xE9; //Jmp
	*(DWORD*)((DWORD)toHook + 1) = relativeAddress; //Jmp Address

	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}

DWORD jumBackAddr;
void __declspec(naked)ourFunct()
{
	__asm
	{
		mov [edi + 0x0000048C], eax
		mov championCode, eax
		jmp [jumBackAddr]
	}
	
}


DWORD WINAPI MainThread(LPVOID param) {
	HMODULE dllH; //Module handle
	HANDLE presentProcessH; //Process handle
	MODULEINFO moduleInfo; //Define a structure to store data
	DWORD moduleAddr; //Module address
	const int hookLength = 6; //Hook length
	DWORD hookAddress; //Hook address

	dllH = GetModuleHandleA("rcp-be-lol-perks.dll"); //Get module handle
	presentProcessH = GetCurrentProcess(); //Get current process handle

	GetModuleInformation(presentProcessH, dllH, &moduleInfo, sizeof(moduleInfo));

	moduleAddr = (DWORD)moduleInfo.lpBaseOfDll; //Get module addr

	hookAddress = moduleAddr+0x37B7D; //Hook address (the address of MOV [EDI+0x48C],EAX
	jumBackAddr = moduleAddr + 0x37B83; //Jump back address

	Hook((void*)hookAddress, ourFunct, hookLength);
	
	return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, MainThread, hModule, 0, 0);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

