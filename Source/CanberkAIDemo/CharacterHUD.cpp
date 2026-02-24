// © 2025 Canberk. All Rights Reserved.

#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CameraAIController.h"
#include "CameraPawn.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "RenderUtils.h"

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

    for (auto& Pair : CameraIndicators)
    {
        if (Pair.Key && Pair.Value.Opacity > 0.01f)
        {
            DrawCameraIndicator(Pair.Key, Pair.Value);
        }
    }
}

void ACharacterHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateIndicatorStates(DeltaTime);

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

void ACharacterHUD::UpdateRockCount(int32 Count)
{
    if (Overlay && Overlay->RockCountText)
    {
        Overlay->RockCountText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Count)));
    }
}

#pragma region Camera Awareness Indicators

void ACharacterHUD::RegisterCamera(ACameraAIController* Camera)
{
    if (Camera && !CameraIndicators.Contains(Camera))
    {
        CameraIndicators.Add(Camera, FCameraIndicatorState());
    }
}

void ACharacterHUD::UnregisterCamera(ACameraAIController* Camera)
{
    CameraIndicators.Remove(Camera);
}

void ACharacterHUD::UpdateIndicatorStates(float DeltaTime)
{
    for (auto& Pair : CameraIndicators)
    {
        ACameraAIController* Cam = Pair.Key;
        FCameraIndicatorState& State = Pair.Value;
        if (!Cam) continue;

        EPlayerAwarenessState Awareness = Cam->GetAwarnessState();
        bool bActive = (Awareness != EPlayerAwarenessState::Unaware && Awareness != EPlayerAwarenessState::ReturningToPatrol);

        float TargetOpacity = bActive ? 1.0f : 0.0f;
        State.Opacity = FMath::FInterpTo(State.Opacity, TargetOpacity, DeltaTime, FadeSpeed);

        if (Awareness == EPlayerAwarenessState::Alerted || Awareness == EPlayerAwarenessState::AlarmTriggered)
        {
            State.PulseTimer += DeltaTime;
        }
        else
        {
            State.PulseTimer = 0.f;
        }
    }
}

void ACharacterHUD::DrawCameraIndicator(ACameraAIController* CameraController, FCameraIndicatorState& IndicatorState)
{
    if (!CameraController || !Canvas) return;

    EPlayerAwarenessState State = CameraController->GetAwarnessState();

    // Get camera world position
    ACameraPawn* CamPawn = Cast<ACameraPawn>(CameraController->GetPawn());
    if (!CamPawn) return;

    FVector WorldPos = CamPawn->GetHeadLocation();

    // Project to screen
    FVector2D ScreenPos;
    FVector CamLoc;
    bool bIsBehindCamera = false;
    ProjectWorldToHUD(WorldPos, ScreenPos, bIsBehindCamera, CamLoc);

    FVector2D ScreenCenter(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);

    // Check if off-screen
    bool bOffScreen = bIsBehindCamera
        || ScreenPos.X < EdgeMargin || ScreenPos.X > ViewportSize.X - EdgeMargin
        || ScreenPos.Y < EdgeMargin || ScreenPos.Y > ViewportSize.Y - EdgeMargin;

    if (bIsBehindCamera)
    {
        ScreenPos = ScreenCenter + (ScreenCenter - ScreenPos);
    }

    if (bOffScreen)
    {
        ScreenPos = ClampToViewportEdge(ScreenCenter, ScreenPos, EdgeMargin);
    }

    // Distance-based scaling (only when on-screen)
    float DistanceScale = 1.0f;
    if (!bOffScreen)
    {
        float Distance = FVector::Dist(CamLoc, WorldPos);
        DistanceScale = FMath::Clamp(1500.f / Distance, 0.6f, 1.5f);
    }

    // Pulse scaling (alert only)
    float PulseScale = 1.0f;
    if (IndicatorState.PulseTimer > 0.f)
    {
        PulseScale = 1.0f + 0.15f * FMath::Sin(IndicatorState.PulseTimer * 8.0f);
    }

    float FinalSize = IndicatorSize * DistanceScale * PulseScale;
    float Opacity = IndicatorState.Opacity;

    // Draw based on state
    float Progress = CameraController->GetSightProgress();

    auto ApplyOpacity = [Opacity](const FLinearColor& Color) -> FLinearColor
    {
        return FLinearColor(Color.R, Color.G, Color.B, Color.A * Opacity);
    };

    switch (State)
    {
    case EPlayerAwarenessState::Suspicious:
    {
        FLinearColor ProgressColor = ApplyOpacity(GetProgressColor(Progress));
        DrawDiamondBorder(ScreenPos, FinalSize, ProgressColor, 2.0f);
        if (Progress > 0.f)
        {
            DrawFilledDiamondPortion(ScreenPos, FinalSize, Progress, ProgressColor);
        }
        break;
    }
    case EPlayerAwarenessState::Investigating:
    {
        FLinearColor Yellow = ApplyOpacity(FLinearColor(1.0f, 0.92f, 0.16f));
        FLinearColor Black = ApplyOpacity(FLinearColor::Black);
        DrawFilledDiamondPortion(ScreenPos, FinalSize, 1.0f, Yellow);
        DrawDiamondBorder(ScreenPos, FinalSize, Yellow, 2.0f);
        DrawCenteredText(TEXT("?"), ScreenPos, Black, 1.0f);
        break;
    }
    case EPlayerAwarenessState::Alerted:
    case EPlayerAwarenessState::AlarmTriggered:
    {
        FLinearColor Red = ApplyOpacity(FLinearColor(1.0f, 0.0f, 0.0f));
        FLinearColor White = ApplyOpacity(FLinearColor::White);
        DrawFilledDiamondPortion(ScreenPos, FinalSize, 1.0f, Red);
        DrawDiamondBorder(ScreenPos, FinalSize, Red, 2.0f);
        DrawCenteredText(TEXT("!"), ScreenPos, White, 1.0f);
        break;
    }
    case EPlayerAwarenessState::LostTarget:
    {
        FLinearColor ProgressColor = ApplyOpacity(GetProgressColor(Progress));
        DrawDiamondBorder(ScreenPos, FinalSize, ProgressColor, 2.0f);
        if (Progress > 0.f)
        {
            DrawFilledDiamondPortion(ScreenPos, FinalSize, Progress, ProgressColor);
        }
        break;
    }
    default:
        break;
    }

    // Arrow pointer when off-screen
    if (bOffScreen)
    {
        FVector2D ArrowDir = (ScreenPos - ScreenCenter);
        ArrowDir.Normalize();
        FLinearColor ArrowColor = ApplyOpacity(GetProgressColor(Progress));
        DrawArrowPointer(ScreenPos, ArrowDir, FinalSize, ArrowColor);
    }
}

bool ACharacterHUD::ProjectWorldToHUD(const FVector& WorldPos, FVector2D& OutScreenPos, bool& bIsBehindCamera, FVector& OutCamLoc) const
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return false;

    FRotator CamRot;
    PC->GetPlayerViewPoint(OutCamLoc, CamRot);

    FVector Dir = WorldPos - OutCamLoc;
    float Dot = FVector::DotProduct(Dir, CamRot.Vector());
    bIsBehindCamera = (Dot < 0.f);

    // Always project (even if behind) so we get a direction to flip
    PC->ProjectWorldLocationToScreen(WorldPos, OutScreenPos, true);

    return true;
}

FVector2D ACharacterHUD::ClampToViewportEdge(const FVector2D& ScreenCenter, const FVector2D& ScreenPos, float Margin) const
{
    FVector2D Dir = ScreenPos - ScreenCenter;
    if (Dir.IsNearlyZero()) return FVector2D(Margin, Margin);

    // Ray-box intersection with viewport rect inset by Margin
    float MinX = Margin;
    float MaxX = ViewportSize.X - Margin;
    float MinY = Margin;
    float MaxY = ViewportSize.Y - Margin;

    float ScaleX = (Dir.X > 0.f) ? (MaxX - ScreenCenter.X) / Dir.X : (Dir.X < 0.f) ? (MinX - ScreenCenter.X) / Dir.X : TNumericLimits<float>::Max();
    float ScaleY = (Dir.Y > 0.f) ? (MaxY - ScreenCenter.Y) / Dir.Y : (Dir.Y < 0.f) ? (MinY - ScreenCenter.Y) / Dir.Y : TNumericLimits<float>::Max();

    float Scale = FMath::Min(ScaleX, ScaleY);
    return ScreenCenter + Dir * Scale;
}

void ACharacterHUD::DrawDiamondBorder(const FVector2D& Center, float Size, const FLinearColor& Color, float Thickness)
{
    FVector2D Top(Center.X, Center.Y - Size);
    FVector2D Right(Center.X + Size, Center.Y);
    FVector2D Bottom(Center.X, Center.Y + Size);
    FVector2D Left(Center.X - Size, Center.Y);

    DrawLine(Top.X, Top.Y, Right.X, Right.Y, Color, Thickness);
    DrawLine(Right.X, Right.Y, Bottom.X, Bottom.Y, Color, Thickness);
    DrawLine(Bottom.X, Bottom.Y, Left.X, Left.Y, Color, Thickness);
    DrawLine(Left.X, Left.Y, Top.X, Top.Y, Color, Thickness);
}

void ACharacterHUD::DrawFilledDiamondPortion(const FVector2D& Center, float Size, float FillPercent, const FLinearColor& Color)
{
    if (FillPercent <= 0.f || !Canvas) return;

    FillPercent = FMath::Clamp(FillPercent, 0.f, 1.f);

    // Diamond vertices
    float Top = Center.Y - Size;
    float Bot = Center.Y + Size;
    float TotalHeight = Bot - Top;

    // Waterline Y: fill from bottom to top
    float WaterlineY = Bot - FillPercent * TotalHeight;

    // Left/Right edge X at a given Y on the diamond
    auto LeftX = [&](float Y) -> float { return Center.X - (Y >= Center.Y ? (Bot - Y) : (Y - Top)); };
    auto RightX = [&](float Y) -> float { return Center.X + (Y >= Center.Y ? (Bot - Y) : (Y - Top)); };

    FCanvasUVTri Tri;
    Tri.V0_UV = Tri.V1_UV = Tri.V2_UV = FVector2D::ZeroVector;
    Tri.V0_Color = Tri.V1_Color = Tri.V2_Color = Color.ToFColor(true);

    TArray<FCanvasUVTri> Triangles;

    if (WaterlineY >= Center.Y)
    {
        // Fill is in the bottom half only
        // Single triangle: Bottom → left edge at waterline → right edge at waterline
        Tri.V0_Pos = FVector2D(Center.X, Bot);
        Tri.V1_Pos = FVector2D(LeftX(WaterlineY), WaterlineY);
        Tri.V2_Pos = FVector2D(RightX(WaterlineY), WaterlineY);
        Triangles.Add(Tri);
    }
    else
    {
        // Full bottom half triangle
        Tri.V0_Pos = FVector2D(Center.X, Bot);
        Tri.V1_Pos = FVector2D(Center.X - Size, Center.Y); // Left vertex
        Tri.V2_Pos = FVector2D(Center.X + Size, Center.Y); // Right vertex
        Triangles.Add(Tri);

        // Top half partial fill: trapezoid from Center.Y to WaterlineY
        // Split into 2 triangles:
        // Tri1: Left@Center.Y → Right@Center.Y → Right@Waterline
        Tri.V0_Pos = FVector2D(Center.X - Size, Center.Y);
        Tri.V1_Pos = FVector2D(Center.X + Size, Center.Y);
        Tri.V2_Pos = FVector2D(RightX(WaterlineY), WaterlineY);
        Triangles.Add(Tri);

        // Tri2: Left@Center.Y → Right@Waterline → Left@Waterline
        Tri.V0_Pos = FVector2D(Center.X - Size, Center.Y);
        Tri.V1_Pos = FVector2D(RightX(WaterlineY), WaterlineY);
        Tri.V2_Pos = FVector2D(LeftX(WaterlineY), WaterlineY);
        Triangles.Add(Tri);
    }

    FCanvasTriangleItem TriItem(Triangles, GWhiteTexture);
    TriItem.SetColor(Color);
    TriItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TriItem);
}

void ACharacterHUD::DrawCenteredText(const FString& Text, const FVector2D& Center, const FLinearColor& Color, float Scale)
{
    if (!Canvas || !GEngine || !GEngine->GetSmallFont()) return;

    UFont* Font = GEngine->GetSmallFont();

    float TextW, TextH;
    Canvas->TextSize(Font, Text, TextW, TextH, Scale);

    FCanvasTextItem TextItem(FVector2D(Center.X - TextW * 0.5f, Center.Y - TextH * 0.5f), FText::FromString(Text), Font, Color);
    TextItem.Scale = FVector2D(Scale, Scale);
    TextItem.bOutlined = false;
    Canvas->DrawItem(TextItem);
}

void ACharacterHUD::DrawArrowPointer(const FVector2D& Center, const FVector2D& Direction, float Size, const FLinearColor& Color)
{
    if (!Canvas) return;

    FVector2D Tip = Center + Direction * (Size + 6.f);
    FVector2D Perp(-Direction.Y, Direction.X);
    FVector2D Base1 = Center + Direction * Size + Perp * 4.f;
    FVector2D Base2 = Center + Direction * Size - Perp * 4.f;

    DrawLine(Tip.X, Tip.Y, Base1.X, Base1.Y, Color, 2.0f);
    DrawLine(Tip.X, Tip.Y, Base2.X, Base2.Y, Color, 2.0f);
    DrawLine(Base1.X, Base1.Y, Base2.X, Base2.Y, Color, 2.0f);
}

FLinearColor ACharacterHUD::GetProgressColor(float Progress) const
{
    Progress = FMath::Clamp(Progress, 0.f, 1.f);

    // Green → Yellow → Orange → Red
    if (Progress < 0.33f)
    {
        float T = Progress / 0.33f;
        return FMath::Lerp(FLinearColor::Green, FLinearColor::Yellow, T);
    }
    else if (Progress < 0.66f)
    {
        float T = (Progress - 0.33f) / 0.33f;
        FLinearColor Orange(1.0f, 0.5f, 0.0f);
        return FMath::Lerp(FLinearColor::Yellow, Orange, T);
    }
    else
    {
        float T = (Progress - 0.66f) / 0.34f;
        FLinearColor Orange(1.0f, 0.5f, 0.0f);
        return FMath::Lerp(Orange, FLinearColor::Red, T);
    }
}

#pragma endregion
