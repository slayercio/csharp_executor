#pragma once
#include <windows.h>
#include <cse/entry.hpp>

DWORD WINAPI MainThread(LPVOID lpParam)
{
    cse::entrypoint();

    FreeLibraryAndExitThread((HMODULE)lpParam, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved)
{
    if (ulReason == DLL_PROCESS_ATTACH)
    {
        HANDLE hThread = CreateThread(NULL, NULL, MainThread, hModule, NULL, NULL);

        if (hThread && hThread != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hThread);
        }
    }

    return TRUE;
}