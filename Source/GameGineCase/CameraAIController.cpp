#include "CameraAIController.h"
#include "CameraPawn.h"
#include <Kismet/GameplayStatics.h>
#include "MissionGameMode.h"
#include "Debugger.h"

ACameraAIController::ACameraAIController()
{
	auto TreeClass = ConstructorHelpers::FObjectFinder<UBehaviorTree>(TEXT("/Game/AI/BT_Camera.BT_Camera"));

	if (TreeClass.Succeeded())
	{
		Tree = TreeClass.Object;
	}
	else
		msg("Failed to find Behavior Tree");

	HearConfig->HearingRange = 800.0f;
	HearConfig->SetMaxAge(5);
	HearConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

	SightConfig->SightRadius = 20000.0f;
	SightConfig->LoseSightRadius = 22000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 20.0f;
	SightConfig->SetMaxAge(5);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComponent->ConfigureSense(*HearConfig);
	PerceptionComponent->ConfigureSense(*SightConfig);
}

void ACameraAIController::BeginPlay() {
	Super::BeginPlay();
}

void ACameraAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location;
    FRotator Rotation;
    GetActorEyesViewPoint(Location, Rotation);
}

void ACameraAIController::OnNoticedPlayer(APawn* Player)
{
	Super::OnNoticedPlayer(Player);

	AMissionGameMode* gm = Cast<AMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (ACameraPawn* Camera = Cast<ACameraPawn>(GetPawn()))
		Camera->ChangeLightColor(FColor(255, 15, 15, 255));
}

void ACameraAIController::OnDetectPlayer()
{
	Super::OnDetectPlayer();
}

void ACameraAIController::OnHeardPlayer(FVector Location)
{
	Super::OnHeardPlayer(Location);
}

void ACameraAIController::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (ACameraPawn* Camera = Cast<ACameraPawn>(GetPawn()))
	{
		OutLocation = Camera->GetHeadLocation();
		OutRotation = Camera->GetHeadRotation();
	}
	else
	{
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}
