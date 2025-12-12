	// © 2024 Gamegine. All Rights Reserved.


#include "RoboAIController.h"


ARoboAIController::ARoboAIController()
{
	auto TreeClass = ConstructorHelpers::FObjectFinder<UBehaviorTree>(TEXT("/Script/AIModule.BehaviorTree'/Game/AI/BT_Robot.BT_Robot'"));

	if (TreeClass.Succeeded())
	{
		Tree = TreeClass.Object;
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, "Failed to find Behavior Tree");

	HearConfig->HearingRange = 800.0f;
	HearConfig->SetMaxAge(5);
	HearConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

	SightConfig->SightRadius = 800.0f;
	SightConfig->LoseSightRadius = 900.0f;
	SightConfig->PeripheralVisionAngleDegrees = 30.0f;
	SightConfig->SetMaxAge(15);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComponent->ConfigureSense(*HearConfig);
	PerceptionComponent->ConfigureSense(*SightConfig);
}

void ARoboAIController::OnNoticedPlayer(APawn* Player)
{

}

void ARoboAIController::OnDetectPlayer()
{
}

void ARoboAIController::OnHeardPlayer(FVector Location)
{

}
