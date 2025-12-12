#include "CameraPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "CameraAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

/*
How this AI will work?
Patrol between two points on the ground in front of it, rotating camera to look at them.
If camera heard or something, look at that point for 3 seconds
If camera saw player, Camera will follow player and a timer will start (5 seconds). If player is still in sight after timer ends trigger alarm


*/

// Sets default values
ACameraPawn::ACameraPawn()
{
    // Enable Tick
    PrimaryActorTick.bCanEverTick = true;

    // Initialize mesh component
    mesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("CameraMesh"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    mesh->SetupAttachment(RootComponent);

    ScannerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Scanner"));
    ScannerLight->SetupAttachment(mesh, FName("CameraJoint"));

    ScannerLight->SetInnerConeAngle(25.f);
    ScannerLight->SetOuterConeAngle(30.f); // match SightConfig->PeripheralVisionAngleDegrees
    ScannerLight->SetAttenuationRadius(800.f); // match SightRadius
    ScannerLight->SetLightColor(FLinearColor::Green); // default scanning color
    ScannerLight->SetIntensity(5000.f);
    ScannerLight->bUseInverseSquaredFalloff = false; // cleaner cone
    ScannerLight->VolumetricScatteringIntensity = 10.0f;

    NotifyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Alert Widget"));
    NotifyWidget->SetupAttachment(RootComponent);
    NotifyWidget->SetRelativeLocation(FVector(0.f,0.f,150.f));
    NotifyWidget->SetWidgetSpace(EWidgetSpace::Screen);
    NotifyWidget->SetDrawSize(FVector2D(100.f, 50.f));
    NotifyWidget->SetVisibility(false);

    ChangeLightColor(FColor(10,150,0,255));
}

void ACameraPawn::BeginPlay()
{
    Super::BeginPlay();

    CalculatePatrolPoints();
    bool isLeftFirst = FMath::RandBool();
    if (isLeftFirst)
        PatrolPoints.Swap(0,1);
    
    TargetPoint = PatrolPoints[0];

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
}

void ACameraPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsPatrolling)
    {
        CurrentPoint = FMath::VInterpTo(CurrentPoint, TargetPoint, DeltaTime, RotateSpeed);
        MoveCamera(CurrentPoint);
    }

}

FTransform ACameraPawn::GetScannerViewTransform() const {
  if(ScannerLight)
    return ScannerLight->GetComponentTransform();

  const FVector Loc = mesh->GetBoneLocationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace); 
  const FRotator Rot = mesh->GetBoneRotationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace);

  return FTransform(Rot, Loc);
}

void ACameraPawn::MoveCameraDelta(FVector TargetLocation, float dt)
{
    FVector CameraLocation = mesh->GetComponentLocation();

    // Calculate the target rotation
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation) + CameraLookOffset;

    // Interpolate towards the target rotation using delta time
    FRotator InterpolatedRotation = FMath::RInterpTo(mesh->GetBoneRotationByName("CameraJoint", EBoneSpaces::WorldSpace), LookAtRotation, dt, RotateSpeed);

    // Set the interpolated rotation to the camera joint
    mesh->SetBoneRotationByName("CameraJoint", InterpolatedRotation, EBoneSpaces::WorldSpace);
}

void ACameraPawn::MoveCamera(FVector TargetLocation)
{
    FVector CameraLocation = mesh->GetComponentLocation();
    
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(CameraLocation, TargetLocation) + CameraLookOffset;

    mesh->SetBoneRotationByName("CameraJoint", LookAtRotation, EBoneSpaces::WorldSpace);
}

void ACameraPawn::MoveCameraToActor(AActor* TargetActor)
{
    if (TargetActor)
    {
        MoveCamera(TargetActor->GetActorLocation());
    }
}

FVector ACameraPawn::GetHeadLocation()
{
    return GetScannerViewTransform().GetLocation(); 
    //return mesh->GetBoneLocationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace);
}

FRotator ACameraPawn::GetHeadRotation()
{
  return GetScannerViewTransform().Rotator();
    //FRotator Rotation = mesh->GetBoneRotationByName(FName("CameraJoint"), EBoneSpaces::WorldSpace);
    //Rotation.Pitch = -40.0f;
    //return Rotation;
}

void ACameraPawn::ChangeLightColor(FColor color)
{
    ScannerLight->SetLightColor(color);
}

void ACameraPawn::SetScannerSight(float NewSightRadius, float NewPeripheralVisionAngleDegrees)
{
    // 1) Update spotlight (visual cone)
    if (ScannerLight)
    {
        ScannerLight->SetAttenuationRadius(NewSightRadius);

        // Treat angle as the same value everywhere so it's visually consistent
        ScannerLight->SetOuterConeAngle(NewPeripheralVisionAngleDegrees);
        ScannerLight->SetInnerConeAngle(FMath::Max(NewPeripheralVisionAngleDegrees - 5.f, 0.f));
    }

    // 2) Update AI sight on the controller
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

    // Calculate the downward rotation by 30 degrees
    FRotator DownwardRotation = CameraRotation + FRotator(RotateAngle, 90.0f, 0.0f);
    FVector ForwardVector = DownwardRotation.Vector();
    FVector LeftVector = FVector::CrossProduct(ForwardVector, FVector::UpVector).GetSafeNormal();

    // Calculate the first patrol point on the ground in front of the camera
    FVector PatrolPoint1 = CameraLocation + ForwardVector * DistanceFromPawn - LeftVector * PointsOffset;
    PatrolPoint1.Z = 0.0f; // Project to ground level

    // Calculate the second patrol point in front of the camera, offset by DistanceBetweenPoints
    FVector PatrolPoint2 = PatrolPoint1 + (LeftVector * DistanceBetweenPoints);
    PatrolPoint2.Z = 0.0f; // Project to ground level

    if (PatrolPoints.Num() > 2)
    {
        PatrolPoints[0] = PatrolPoint1;
        PatrolPoints[1] = PatrolPoint2;
    }
    else {
        PatrolPoints.Add(PatrolPoint1);
        PatrolPoints.Add(PatrolPoint2);
    }

    UE_LOG(LogTemp, Error, TEXT("Patrol points calculated: %d, Points: (%s, %s)"), PatrolPoints.Num(), *PatrolPoint1.ToString(), *PatrolPoint2.ToString());
}

float ACameraPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float Dealt = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    if (Dealt > Health)
    {
        if (ExplosionEffect)
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ExplosionEffect,GetTransform(),true);
        if (ExplosionSound) {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(),ExplosionSound,GetActorLocation(),4.0f);
            MakeNoise(3,this,GetActorLocation());
        }
        Destroy();
    }

    return Dealt;
}
