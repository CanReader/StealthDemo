#include "CameraPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "CameraAIController.h"
#include "AlertWidget.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ACameraPawn::ACameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    mesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("CameraMesh"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    mesh->SetupAttachment(RootComponent);

    ScannerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Scanner"));
    ScannerLight->SetupAttachment(mesh, FName("CameraJoint"));

    ScannerLight->SetInnerConeAngle(25.f);
    ScannerLight->SetOuterConeAngle(30.f);
    ScannerLight->SetAttenuationRadius(800.f);
    ScannerLight->SetLightColor(FLinearColor::Green);
    ScannerLight->SetIntensity(5000.f);
    ScannerLight->bUseInverseSquaredFalloff = false;
    ScannerLight->VolumetricScatteringIntensity = 10.0f;

    NotifyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Alert Widget"));
    NotifyWidget->SetupAttachment(RootComponent);
    NotifyWidget->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
    NotifyWidget->SetWidgetSpace(EWidgetSpace::Screen);
    NotifyWidget->SetDrawSize(FVector2D(100.f, 50.f));
    NotifyWidget->SetVisibility(false);

    ServoAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ServoAudio"));
    ServoAudioComponent->SetupAttachment(mesh, FName("CameraJoint"));
    ServoAudioComponent->bAutoActivate = false;

    FLinearColor InitialColor = FColor(10, 150, 0, 255);
    CurrentLightColor = InitialColor;
    TargetLightColor = InitialColor;
    ScannerLight->SetLightColor(InitialColor);
}

void ACameraPawn::BeginPlay()
{
    Super::BeginPlay();

    CalculatePatrolPoints();
    bool isLeftFirst = FMath::RandBool();
    if (isLeftFirst)
        PatrolPoints.Swap(0, 1);

    // Initialize look target and bone rotation to first patrol point
    CurrentPatrolIndex = 0;
    DesiredLookTarget = PatrolPoints[0];

    FVector CameraLocation = mesh->GetComponentLocation();
    CurrentBoneRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, DesiredLookTarget) + CameraLookOffset;
    mesh->SetBoneRotationByName("CameraJoint", CurrentBoneRotation, EBoneSpaces::WorldSpace);

    if (ACameraAIController* CamController = Cast<ACameraAIController>(GetController()))
    {
        if (CamController->GetSightConfig())
        {
            SetScannerSight(
                CamController->GetSightConfig()->SightRadius,
                CamController->GetSightConfig()->PeripheralVisionAngleDegrees
            );
        }
    }

    // Start servo loop sound if assigned
    if (ServoLoopCue && ServoAudioComponent)
    {
        ServoAudioComponent->SetSound(ServoLoopCue);
        ServoAudioComponent->Play();
    }
}

void ACameraPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Priority 1: Follow a target actor
    if (FollowTargetActor)
    {
        DesiredLookTarget = FollowTargetActor->GetActorLocation();
    }
    // Priority 2: If patrolling, update patrol logic
    else if (bIsPatrolling)
    {
        UpdatePatrol(DeltaTime);
    }
    // Priority 3: DesiredLookTarget was set externally (by BT task via SetLookTarget)

    UpdateBoneRotation(DeltaTime);
    UpdateLightColor(DeltaTime);
    UpdateLightIntensity(DeltaTime);
}

void ACameraPawn::UpdateBoneRotation(float DeltaTime)
{
    FVector CameraLocation = mesh->GetComponentLocation();
    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(CameraLocation, DesiredLookTarget) + CameraLookOffset;

    // Servo settling: slow down in final 5 degrees for realistic deceleration
    float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(CurrentBoneRotation.Yaw - TargetRot.Yaw));
    const float SettleThreshold = 5.0f;
    const float MinSpeedFraction = 0.3f;
    float EffectiveSpeed = RotateSpeed;
    if (AngleDiff < SettleThreshold)
    {
        float T = AngleDiff / SettleThreshold;
        EffectiveSpeed = RotateSpeed * FMath::Lerp(MinSpeedFraction, 1.0f, T);
    }

    CurrentBoneRotation = FMath::RInterpTo(CurrentBoneRotation, TargetRot, DeltaTime, EffectiveSpeed);
    mesh->SetBoneRotationByName("CameraJoint", CurrentBoneRotation, EBoneSpaces::WorldSpace);

    // Modulate servo pitch based on rotation speed
    if (ServoAudioComponent && ServoAudioComponent->IsPlaying())
    {
        float NormalizedSpeed = FMath::Clamp(AngleDiff / 30.0f, 0.0f, 1.0f);
        ServoAudioComponent->SetPitchMultiplier(FMath::Lerp(0.6f, 1.2f, NormalizedSpeed));
        ServoAudioComponent->SetVolumeMultiplier(FMath::Lerp(0.3f, 1.0f, NormalizedSpeed));
    }
}

void ACameraPawn::UpdatePatrol(float DeltaTime)
{
    if (PatrolPoints.Num() < 2) return;

    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.f)
        {
            bWaitingAtPatrolPoint = false;
            // Move to next patrol point
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        }
        return;
    }

    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
    DesiredLookTarget = TargetPoint;

    // Check if we've roughly reached the look target (rotation is close enough)
    FVector CameraLocation = mesh->GetComponentLocation();
    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetPoint) + CameraLookOffset;
    float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(CurrentBoneRotation.Yaw - TargetRot.Yaw));

    if (AngleDiff < 2.0f)
    {
        bWaitingAtPatrolPoint = true;
        PatrolWaitTimer = PatrolWaitTime;
    }
}

FTransform ACameraPawn::GetScannerViewTransform() const
{
    if (ScannerLight)
        return ScannerLight->GetComponentTransform();

    const FVector Loc = mesh->GetBoneLocationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace);
    const FRotator Rot = mesh->GetBoneRotationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace);
    return FTransform(Rot, Loc);
}

FVector ACameraPawn::GetHeadLocation()
{
    return GetScannerViewTransform().GetLocation();
}

FRotator ACameraPawn::GetHeadRotation()
{
    return GetScannerViewTransform().Rotator();
}

void ACameraPawn::ChangeLightColor(FColor color)
{
    TargetLightColor = FLinearColor(color);
}

void ACameraPawn::UpdateLightColor(float DeltaTime)
{
    if (!CurrentLightColor.Equals(TargetLightColor, 0.001f))
    {
        CurrentLightColor = FLinearColor::LerpUsingHSV(CurrentLightColor, TargetLightColor, FMath::Clamp(DeltaTime * LightColorInterpSpeed, 0.f, 1.f));
        ScannerLight->SetLightColor(CurrentLightColor);
    }
}

void ACameraPawn::UpdateLightIntensity(float DeltaTime)
{
    PulseTimer += DeltaTime;
    float Pulse = BaseLightIntensity + PulseAmplitude * FMath::Sin(PulseTimer * PulseFrequency * 2.0f * PI);
    ScannerLight->SetIntensity(Pulse);
}

void ACameraPawn::SetLightIntensityMode(float InBaseIntensity, float InPulseAmplitude, float InPulseFrequency)
{
    BaseLightIntensity = InBaseIntensity;
    PulseAmplitude = InPulseAmplitude;
    PulseFrequency = InPulseFrequency;
}

void ACameraPawn::PlayNoticeSound()
{
    if (NoticeSoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), NoticeSoundCue, GetActorLocation());
    }
}

void ACameraPawn::PlayAlertSound()
{
    if (AlertSoundCue)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), AlertSoundCue, GetActorLocation());
    }
}

#pragma region Smooth Movement API

void ACameraPawn::SetLookTarget(FVector InTarget)
{
    DesiredLookTarget = InTarget;
}

void ACameraPawn::SetFollowTarget(AActor* InActor)
{
    FollowTargetActor = InActor;
}

void ACameraPawn::ClearFollowTarget()
{
    FollowTargetActor = nullptr;
}

void ACameraPawn::ResumePatrol()
{
    bIsPatrolling = true;
    bWaitingAtPatrolPoint = false;
}

void ACameraPawn::PausePatrol()
{
    bIsPatrolling = false;
}

UAlertWidget* ACameraPawn::GetNotifyWidget() const
{
    if (NotifyWidget)
    {
        return Cast<UAlertWidget>(NotifyWidget->GetUserWidgetObject());
    }
    return nullptr;
}

void ACameraPawn::SetNotifyWidgetVisible(bool bVisible)
{
    if (NotifyWidget)
    {
        NotifyWidget->SetVisibility(bVisible);
    }
}

#pragma endregion

void ACameraPawn::SetScannerSight(float NewSightRadius, float NewPeripheralVisionAngleDegrees)
{
    if (ScannerLight)
    {
        ScannerLight->SetAttenuationRadius(NewSightRadius);
        ScannerLight->SetOuterConeAngle(NewPeripheralVisionAngleDegrees);
        ScannerLight->SetInnerConeAngle(FMath::Max(NewPeripheralVisionAngleDegrees - 5.f, 0.f));
    }

    if (ACameraAIController* CamController = Cast<ACameraAIController>(GetController()))
    {
        if (auto config = CamController->GetSightConfig())
        {
            config->SightRadius = NewSightRadius;
            config->LoseSightRadius = NewSightRadius * 1.5;
            config->PeripheralVisionAngleDegrees = NewPeripheralVisionAngleDegrees;
            CamController->SetSightConfig(config);

            if (CamController->PerceptionComponent)
            {
                CamController->PerceptionComponent->RequestStimuliListenerUpdate();
            }
        }
    }
}

void ACameraPawn::CalculatePatrolPoints()
{
    FVector CameraLocation = GetActorLocation();
    FRotator CameraRotation = GetActorRotation();

    FRotator DownwardRotation = CameraRotation + FRotator(RotateAngle, 90.0f, 0.0f);
    FVector ForwardVector = DownwardRotation.Vector();
    FVector LeftVector = FVector::CrossProduct(ForwardVector, FVector::UpVector).GetSafeNormal();

    FVector PatrolPoint1 = CameraLocation + ForwardVector * DistanceFromPawn - LeftVector * PointsOffset;
    PatrolPoint1.Z = 0.0f;

    FVector PatrolPoint2 = PatrolPoint1 + (LeftVector * DistanceBetweenPoints);
    PatrolPoint2.Z = 0.0f;

    if (PatrolPoints.Num() > 2)
    {
        PatrolPoints[0] = PatrolPoint1;
        PatrolPoints[1] = PatrolPoint2;
    }
    else
    {
        PatrolPoints.Add(PatrolPoint1);
        PatrolPoints.Add(PatrolPoint2);
    }

    UE_LOG(LogTemp, Log, TEXT("Patrol points calculated: %d, Points: (%s, %s)"), PatrolPoints.Num(), *PatrolPoint1.ToString(), *PatrolPoint2.ToString());
}

float ACameraPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float Dealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (Dealt > Health)
    {
        if (ExplosionEffect)
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetTransform(), true);
        if (ExplosionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation(), 4.0f);
            MakeNoise(3, this, GetActorLocation());
        }
        Destroy();
    }

    return Dealt;
}
