#include "pch.h"
#include "GameMode.h"
#include "Utils.h"
#include "Misc.h"

bool GameMode::ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
    AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState);
    if (GameState->MapInfo == NULL)
        return false;

    static FName WaitingToStart = UKismetStringLibrary::Conv_StringToName(L"WaitingToStart");

    if (!GameMode->bWorldIsReady)
    {
        FURL URL;
        URL.Port = 7777;

        if (!Misc::Listen(UWorld::GetWorld(), &URL))
            return false;

        GameMode->bWorldIsReady = true;
        SetConsoleTitleA("Winter | Ready");
    }

    if (GameMode->MatchState == UKismetStringLibrary::Conv_StringToName(TEXT("WaitingToStart")))
    {
        if (GameMode->NumPlayers > 0)
        {
            return true;
        }
    }

    return false;
}

APawn* GameMode::SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot) {
    return GameMode->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform());
}

void GameMode::Patch()
{
    Utils::Virtual<AFortGameModeAthena>(0xC5, SpawnDefaultPawnFor);
    Utils::Virtual<AFortGameModeAthena>(0xFE, ReadyToStartMatch);
}