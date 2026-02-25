// © 2025-2026 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LevelSaveRepository.generated.h"


/**
 * 
 */
UCLASS()
class CANBERKAIDEMO_API ULevelSaveRepository : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Saver")
	static void SaveGame(bool bLevel1Completed, bool bLevel2Completed, bool bLevel3Completed);

	UFUNCTION(BlueprintCallable, Category = "Loader")
	static void LoadGame(bool& bLevel1Completed, bool& bLevel2Completed, bool& bLevel3Completed);

private:

};
