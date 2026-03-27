#include "pch.h"
#include "Misc.h"
#include "GameMode.h"
#include "Abilities.h"
#include "Player.h"
#include "Bots.h"
#include "Utils.h"

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

    Abilities::Setup();
    Misc::Setup();
    GameMode::Setup();
    Abilities::Setup();
    Player::Setup();
    Bots::Setup();

    *(bool*)(InSDKUtils::GetImageBase() + 0x6F41270) = false; // GIsClient
    //*(bool*)(InSDKUtils::GetImageBase() + 0x6F41270 + 1) = true; // GIsServer

    *(int32*)(InSDKUtils::GetImageBase() + 0x6E82A82) = 1; // Phoebe.Minimap

    *(int32*)(InSDKUtils::GetImageBase() + 0x6E82A83) = 1; // Phoebe.OverrideMMR
    *(int32*)(InSDKUtils::GetImageBase() + 0x680F388) = 70; // Phoebe.MaxSpawnAmount

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"log LogAthenaBots VeryVerbose", nullptr);

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