// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h" 
#include "Components/ProgressBar.h" 
#include "Components/HorizontalBox.h"
#include "CharacterOverlay.generated.h"

UCLASS()
class GAMEGINECASE_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
		UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* AmmoText;
};
