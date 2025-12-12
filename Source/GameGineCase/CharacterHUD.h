// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterOverlay.h"
#include "CharacterHUD.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	virtual void Tick(float DetaTime) override;

	void UpdateHealth(float value);
	void UpdateAmmo(int32 Current, int32 Carried);
	UFUNCTION(BlueprintCallable)
	void ShowHealthBar(float Speed);
	UFUNCTION(BlueprintCallable)
	void ShowAmmoBar(float Speed);

	UFUNCTION(BlueprintCallable)
	void HideHealthBar(float Speed);
	UFUNCTION(BlueprintCallable)
	void HideAmmoBar(float Speed);
private:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> OverlayClass;

	TObjectPtr<UCharacterOverlay> Overlay;
	FVector2D ViewportSize;
	float VisibilitySpeed;

	bool bIsHealthVisible = false;
	bool bIsAmmoVisible = false;
public:
	FORCEINLINE UCharacterOverlay* GetOverlay() {
		return Overlay;
	}
};
