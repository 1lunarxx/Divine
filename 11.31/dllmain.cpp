#include "pch.h"
#include "Misc.h"
#include "GameMode.h"
#include "Abilities.h"
#include "Player.h"
#include "Bots.h"
#include "Utils.h"

static inline void NYevent()
{
    auto NYT = Utils::StaticFindObject<UObject>("/NewYears/Content/Levels/Apollo_NYE_Celebration.Apollo_NYE_Celebration.PersistentLevel.BP_NewYearTimer_2");;

    auto NYE = Utils::StaticFindObject<UFunction>("/NewYears/Blueprints/BP_NewYearTimer.BP_NewYearTimer_C.startNYE");

    if (NYT && NYE)
    {
        NYT->ProcessEvent(NYE, nullptr);

        printf("NYE: \n");
    }
}

DWORD WINAPI LaunchWindowsStartup(LPVOID)
{
#ifdef CLIENT
    UEngine::GetEngine()->GameViewport->ViewportConsole = (UConsole*)UGameplayStatics::SpawnObject(UEngine::GetEngine()->ConsoleClass, UEngine::GetEngine()->GameViewport);
    *(uint8*)(InSDKUtils::GetImageBase() + 0x6E82A82) = 1;
    return 0;
#endif

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
    Bots::Patch();
    NYevent();

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