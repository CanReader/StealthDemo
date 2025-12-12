// © 2024 Gamegine. All Rights Reserved.

#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"

void ACharacterHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* Controller = GetOwningPlayerController();

    if (Controller && OverlayClass)
    {
        Overlay = CreateWidget<UCharacterOverlay>(Controller, OverlayClass);
        Overlay->AddToViewport();
        Overlay->HealthBar->SetRenderOpacity(0.0f);
        Overlay->AmmoText->SetRenderOpacity(0.0f);

        ShowHealthBar(15);
        ShowAmmoBar(15);
    }
}

void ACharacterHUD::DrawHUD()
{
    Super::DrawHUD();

    GEngine->GameViewport->GetViewportSize(ViewportSize);
}

void ACharacterHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Overlay)
    {
        float HealthTarget = bIsHealthVisible ? 1.0f : 0.0f;
        float AmmoTarget = bIsAmmoVisible ? 1.0f : 0.0f;

        float tHealth = FMath::FInterpTo(Overlay->HealthBar->GetRenderOpacity(), HealthTarget, DeltaTime, VisibilitySpeed);
        float tAmmo = FMath::FInterpTo(Overlay->AmmoText->GetRenderOpacity(), AmmoTarget, DeltaTime, VisibilitySpeed);

        Overlay->HealthBar->SetRenderOpacity(tHealth);
        Overlay->AmmoText->SetRenderOpacity(tAmmo);
    }
}

void ACharacterHUD::ShowHealthBar(float Speed)
{
    if (Overlay)
    {
        Overlay->HealthBar->SetRenderOpacity(0.0f);
        VisibilitySpeed = Speed;
        bIsHealthVisible = true;

    }
}

void ACharacterHUD::ShowAmmoBar(float Speed)
{
    if (Overlay) {
        Overlay->AmmoText->SetRenderOpacity(0.0f);
        VisibilitySpeed = Speed;
        bIsAmmoVisible = true;
    }
}

void ACharacterHUD::HideHealthBar(float Speed)
{
    if (Overlay) {
        Overlay->HealthBar->SetRenderOpacity(1.0);
        VisibilitySpeed = Speed;
        bIsHealthVisible = false;
    }
}

void ACharacterHUD::HideAmmoBar(float Speed)
{
    if (Overlay) {
        Overlay->AmmoText->SetRenderOpacity(1.0);
        VisibilitySpeed = Speed;
        bIsAmmoVisible = false;
    }
}

void ACharacterHUD::UpdateHealth(float Value)
{
    if (Overlay)
        Overlay->HealthBar->SetPercent(Value / 100.0f);
}

void ACharacterHUD::UpdateAmmo(int32 Current, int32 Carried)
{
    if (Overlay)
    Overlay->AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), Current, Carried)));
}
