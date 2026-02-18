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
		HearConfig->SetMaxAge(HeardForgetTime);
		HearConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

		SightConfig->SightRadius = 20000.0f;
		SightConfig->LoseSightRadius = 22000.0f;
		SightConfig->PeripheralVisionAngleDegrees = 20.0f;
		SightConfig->SetMaxAge(SightForgetTime);
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		PerceptionComponent->ConfigureSense(*HearConfig);
		PerceptionComponent->ConfigureSense(*SightConfig);

	}

	void ACameraAIController::BeginPlay() {
		Super::BeginPlay();

		camera = GetPawn<ACameraPawn>();

		SetAwarnessState(EPlayerAwarenessState::Unaware);
	}

	void ACameraAIController::Tick(float DeltaTime)
	{
		Super::Tick(DeltaTime);

		FVector Location;
		FRotator Rotation;
		GetActorEyesViewPoint(Location, Rotation);
	}

	void ACameraAIController::SetAwarnessState(EPlayerAwarenessState state)
	{
		Super::SetAwarnessState(state);

		if(camera)
			camera->ChangeLightColor(
				state == EPlayerAwarenessState::Alerted ? FColor::FromHex("A70004FF") :
				state == EPlayerAwarenessState::LostTarget ? FColor::FromHex("FFA500FF") :  
				(state == EPlayerAwarenessState::Suspicious ? FColor::FromHex("FFED29FF") :
					FColor::FromHex("82E600FF")));
	}

	void ACameraAIController::OnLostPlayer(bool bIsSightLost)
	{
		Super::OnLostPlayer(bIsSightLost);
	}

	void ACameraAIController::OnNoticedPlayer(APawn* Player)
	{
		Super::OnNoticedPlayer(Player);
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
