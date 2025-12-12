// © 2024 Gamegine. All Rights Reserved.
// Author: Canberk Pitirli
// Project: GameGine Case Project
// Description: Defines the save game functionality to track and store the completion status of levels. Provides methods to save and retrieve level completion data, ensuring player progress is preserved across sessions.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LevelSave.generated.h"

/**
 *
 */
UCLASS()
class GAMEGINECASE_API ULevelSave : public USaveGame
{
	GENERATED_BODY()

public:
	ULevelSave();

	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool bLevel1Completed;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool bLevel2Completed;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool bLevel3Completed;

};
