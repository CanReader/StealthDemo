// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterOverlay.h"
#include "CharacterHUD.generated.h"

class ACameraAIController;

USTRUCT()
struct FCameraIndicatorState
{
	GENERATED_BODY()

	float Opacity = 0.f;
	float PulseTimer = 0.f;
};

UCLASS()
class CANBERKAIDEMO_API ACharacterHUD : public AHUD
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

	void RegisterCamera(ACameraAIController* Camera);
	void UnregisterCamera(ACameraAIController* Camera);

private:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> OverlayClass;

	TObjectPtr<UCharacterOverlay> Overlay;
	FVector2D ViewportSize;
	float VisibilitySpeed;

	bool bIsHealthVisible = false;
	bool bIsAmmoVisible = false;

#pragma region Camera Awareness Indicators
	UPROPERTY()
	TMap<ACameraAIController*, FCameraIndicatorState> CameraIndicators;

	void UpdateIndicatorStates(float DeltaTime);
	void DrawCameraIndicator(ACameraAIController* CameraController, FCameraIndicatorState& State);
	bool ProjectWorldToHUD(const FVector& WorldPos, FVector2D& OutScreenPos, bool& bIsBehindCamera, FVector& OutCamLoc) const;
	FVector2D ClampToViewportEdge(const FVector2D& ScreenCenter, const FVector2D& ScreenPos, float Margin) const;
	void DrawDiamondBorder(const FVector2D& Center, float Size, const FLinearColor& Color, float Thickness);
	void DrawFilledDiamondPortion(const FVector2D& Center, float Size, float FillPercent, const FLinearColor& Color);
	void DrawCenteredText(const FString& Text, const FVector2D& Center, const FLinearColor& Color, float Scale);
	void DrawArrowPointer(const FVector2D& Center, const FVector2D& Direction, float Size, const FLinearColor& Color);
	FLinearColor GetProgressColor(float Progress) const;

	float IndicatorSize = 20.f;
	float EdgeMargin = 40.f;
	float FadeSpeed = 8.f;
#pragma endregion

public:
	FORCEINLINE UCharacterOverlay* GetOverlay() {
		return Overlay;
	}
};
