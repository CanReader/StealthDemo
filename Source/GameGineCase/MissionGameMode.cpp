// © 2024 Gamegine. All Rights Reserved.


#include "MissionGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "AgentCharacter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/AssetManager.h"

bool AMissionGameMode::bHasPlayedInitialSound = false;

AMissionGameMode::AMissionGameMode() : LevelNames({ "Level_1","Level_2","Level_3" })
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_AgentCharacter"));
    if (PlayerPawnBPClass.Class != NULL)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}

void AMissionGameMode::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* player = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (player)
    {
        character = Cast<AAgentCharacter>(player->GetPawn());
    }

    FString LevelName = GetWorld()->GetMapName();
    LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

    // Store the level value to use later
    if ((CurrentLevel = LevelNames.IndexOfByKey(LevelName)) == INDEX_NONE)
    {
        UE_LOG(LogGameMode, Error, TEXT("Failed to find level"));
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
        return;
    }
    else
    {
        UE_LOG(LogGameMode, Log, TEXT("The %s loaded!"), *LevelName);
    }

    // Indices start at 0, increase is needed here
    CurrentLevel += 1;

    FString SoundPath;

    // Get level options from command line parameters
    FString Options = FCommandLine::Get();
    FString Option = UGameplayStatics::ParseOption(Options, TEXT("restart"));

    UE_LOG(LogTemp, Warning, TEXT("The options of the level: %s"), *Options);

    // Use welcome sound, otherwise play failed sound
    if (Option != "true")
    { //TODO: Options works on production builds! So this code will work only in production
        SoundPath = FString::Printf(TEXT("/Script/Engine.SoundWave'/Game/Audio/SOUND_Mission%d_Welcome.SOUND_Mission%d_Welcome'"), CurrentLevel, CurrentLevel);
    }
    else
    {
        SoundPath = FString::Printf(TEXT("/Script/Engine.SoundWave'/Game/Audio/SOUND_FailRestart.SOUND_FailRestart'"));
    }

    // Get Asset Registry module to get the assets in runtime and try to find the requested sound asset
    auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    FAssetData SoundAssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*SoundPath);

    if (SoundAssetData.IsValid())
    {
        // Play the sound if valid
        USoundWave* Sound = Cast<USoundWave>(SoundAssetData.GetAsset());
        if (Sound && bInitialSoundCanPlay)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), Sound);
        }
        else
        {
            UE_LOG(LogGameMode, Error, TEXT("Could not find the sound"));
        }
    }
    else
    {
        UE_LOG(LogGameMode, Error, TEXT("Failed to load sound: %s"), *SoundPath);
    }
}


void AMissionGameMode::SpawnNPCs()
{
}

void AMissionGameMode::RestartMission()
{
    FString LevelName = FString::Printf(TEXT("Level_%d"), CurrentLevel);
    UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), true, FString("restart=true"));
}

void AMissionGameMode::CompleteMission()
{
    if (CurrentLevel != 3)
    {
        FString LevelName = FString::Printf(TEXT("Level_%d"), CurrentLevel + 1);
        UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), false);
    }
    else
    {   
        UGameplayStatics::OpenLevel(GetWorld(), FName("GameCompletion"), true);
    }
}

void AMissionGameMode::SetEnableInitialSound(bool value) {
    this->bInitialSoundCanPlay = value;
}

