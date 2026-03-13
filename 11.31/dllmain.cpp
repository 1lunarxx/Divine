#include "pch.h"

DWORD WINAPI LaunchWindowsStartup(LPVOID)
{
    Sleep(5000);
    AllocConsole();
    SetConsoleTitleA("Winter | Loading");
    MH_Initialize();

    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    UGameplayStatics::OpenLevel(UWorld::GetWorld(), UKismetStringLibrary::Conv_StringToName(L"Apollo_Terrain"), false, FString());

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(NULL, 0, LaunchWindowsStartup, NULL, 0, NULL);

    return TRUE;
}