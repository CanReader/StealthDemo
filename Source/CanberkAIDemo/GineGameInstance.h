// © 2025-2026 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GineGameInstance.generated.h"

/**
 *
 */
UCLASS()
class CANBERKAIDEMO_API UCanberkGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Game State")
	bool bHasPlayedInitialSound = false;
};
