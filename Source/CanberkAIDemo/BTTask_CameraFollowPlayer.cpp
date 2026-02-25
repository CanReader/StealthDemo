// © 2025-2026 Canberk. All Rights Reserved.

#include "BTTask_CameraFollowPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CameraPawn.h"
#include "CameraAIController.h"

UBTTask_CameraFollowPlayer::UBTTask_CameraFollowPlayer()
{
	NodeName = "Camera Follow Player";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_CameraFollowPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());
	if (!Camera) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AActor* PlayerActor = Cast<AActor>(BB->GetValueAsObject(FName("Character")));
	if (!PlayerActor) return EBTNodeResult::Failed;

	CurrentPhase = ECameraFollowPhase::Following;
	PhaseTimer = 0.f;
	CurrentInvestigateIndex = 0;
	InvestigatePoints.Empty();

	Camera->PausePatrol();
	Camera->SetFollowTarget(PlayerActor);

	return EBTNodeResult::InProgress;
}

void UBTTask_CameraFollowPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACameraAIController* CamAI = Cast<ACameraAIController>(AIController);
	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!Camera || !BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check for alert at any phase
	if (CamAI && CamAI->GetAwarnessState() == EPlayerAwarenessState::Alerted)
	{
		Camera->ClearFollowTarget();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	switch (CurrentPhase)
	{
	case ECameraFollowPhase::Following:
	{
		bool bCanSee = BB->GetValueAsBool(FName("bDidSee"));
		if (!bCanSee)
		{
			Camera->ClearFollowTarget();
			LastSeenLocation = BB->GetValueAsVector(FName("LastSeenLocation"));
			Camera->SetLookTarget(LastSeenLocation);
			PhaseTimer = 0.f;
			CurrentPhaseDuration = LookAtLastSeenDuration + FMath::FRandRange(-DurationVariance, DurationVariance);
			CurrentPhase = ECameraFollowPhase::LookingAtLastSeen;

			if (CamAI)
			{
				CamAI->SetAwarnessState(EPlayerAwarenessState::Investigating);
			}
		}
		break;
	}

	case ECameraFollowPhase::LookingAtLastSeen:
	{
		// If player is seen again during investigation → immediate alert
		bool bCanSee = BB->GetValueAsBool(FName("bDidSee"));
		if (bCanSee)
		{
			if (CamAI) CamAI->TriggerAlert();
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		PhaseTimer += DeltaSeconds;
		if (PhaseTimer >= CurrentPhaseDuration)
		{
			EnterInvestigatePhase(OwnerComp);
		}
		break;
	}

	case ECameraFollowPhase::Investigating:
	{
		// If player is seen again during investigation → immediate alert
		bool bCanSee = BB->GetValueAsBool(FName("bDidSee"));
		if (bCanSee)
		{
			if (CamAI) CamAI->TriggerAlert();
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		PhaseTimer += DeltaSeconds;
		if (PhaseTimer >= CurrentPhaseDuration)
		{
			CurrentInvestigateIndex++;
			if (CurrentInvestigateIndex >= InvestigatePoints.Num())
			{
				// Investigation complete, return to patrol
				if (CamAI)
				{
					CamAI->SetAwarnessState(EPlayerAwarenessState::Unaware);
				}
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}

			Camera->SetLookTarget(InvestigatePoints[CurrentInvestigateIndex]);
			PhaseTimer = 0.f;
			CurrentPhaseDuration = InvestigatePointDuration + FMath::FRandRange(-DurationVariance, DurationVariance);
		}
		break;
	}
	}
}

void UBTTask_CameraFollowPlayer::EnterInvestigatePhase(UBehaviorTreeComponent& OwnerComp)
{
	CurrentPhase = ECameraFollowPhase::Investigating;
	PhaseTimer = 0.f;
	CurrentPhaseDuration = InvestigatePointDuration + FMath::FRandRange(-DurationVariance, DurationVariance);
	CurrentInvestigateIndex = 0;
	InvestigatePoints.Empty();

	AAIController* AIController = OwnerComp.GetAIOwner();
	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());

	FVector CameraLocation = Camera->GetActorLocation();
	FVector ForwardDir = (LastSeenLocation - CameraLocation).GetSafeNormal2D();

	for (int32 i = 0; i < InvestigatePointCount; i++)
	{
		FVector RandomPoint = FVector::ZeroVector;
		// Try up to 10 times to find a point in the forward hemisphere
		for (int32 Attempt = 0; Attempt < 10; Attempt++)
		{
			FVector RandomOffset = FMath::VRand() * FMath::RandRange(100.f, InvestigateRadius);
			RandomOffset.Z = 0.f;
			FVector Candidate = LastSeenLocation + RandomOffset;
			FVector DirToCandidate = (Candidate - CameraLocation).GetSafeNormal2D();

			if (FVector::DotProduct(ForwardDir, DirToCandidate) > 0.f)
			{
				RandomPoint = Candidate;
				break;
			}
		}

		// Fallback if all attempts were behind
		if (RandomPoint.IsZero())
		{
			FVector RandomOffset = ForwardDir.RotateAngleAxis(FMath::RandRange(-70.f, 70.f), FVector::UpVector) * FMath::RandRange(100.f, InvestigateRadius);
			RandomPoint = LastSeenLocation + RandomOffset;
		}

		InvestigatePoints.Add(RandomPoint);
	}

	if (InvestigatePoints.Num() > 0)
	{
		Camera->SetLookTarget(InvestigatePoints[0]);
	}
}

EBTNodeResult::Type UBTTask_CameraFollowPlayer::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		if (ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn()))
		{
			Camera->ClearFollowTarget();
		}
	}

	return EBTNodeResult::Aborted;
}

FString UBTTask_CameraFollowPlayer::GetStaticDescription() const
{
	return FString::Printf(TEXT("Follow player, then investigate: look at last seen for %.1fs, then %d points x %.1fs"),
		LookAtLastSeenDuration, InvestigatePointCount, InvestigatePointDuration);
}
