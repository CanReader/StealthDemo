// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GineGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API UGineGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game State") 
	bool bHasPlayedInitialSound = false;
};
