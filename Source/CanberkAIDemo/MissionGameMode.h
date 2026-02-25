// © 2025-2026 Canberk. All Rights Reserved.
// Author: Canberk Pitirli
// Project: CanberkAIDemo
// Description: Defines the mission-based game mode, tracking objectives and player progress. The mission is completed when the player successfully reaches the designated exit area, ensuring a clear and achievable goal for each gameplay session.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MissionGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CANBERKAIDEMO_API AMissionGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMissionGameMode();
	virtual void BeginPlay() override;
	
	void SpawnNPCs();
	UFUNCTION(BlueprintCallable)
	void RestartMission();
	UFUNCTION(BlueprintCallable)
	void CompleteMission();
	UFUNCTION(BlueprintCallable)
	void SetEnableInitialSound(bool value);

private:
	class AAgentCharacter* character;
	TArray<FString> LevelNames;
	int CurrentLevel;
	
	static bool bHasPlayedInitialSound;
	bool bInitialSoundCanPlay;
public:
	FORCEINLINE int GetCurrentLevel() { return CurrentLevel; }
};
