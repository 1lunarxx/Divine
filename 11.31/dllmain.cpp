#include "pch.h"
#include "Misc.h"
#include "GameMode.h"
#include "Abilities.h"
#include "Player.h"

DWORD WINAPI LaunchWindowsStartup(LPVOID)
{
    Sleep(5000);
    AllocConsole();
    SetConsoleTitleA("Winter | Loading");

    FILE* F = nullptr;
    freopen_s(&F, "CONIN$", "r", stdin);
    freopen_s(&F, "CONOUT$", "w", stdout);
    freopen_s(&F, "CONOUT$", "w", stderr);

    MH_Initialize();

    Misc::Patch();
    GameMode::Patch();
    Abilities::Patch();
    Player::Patch();

    *(bool*)(InSDKUtils::GetImageBase() + 0x6F41270) = false;

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