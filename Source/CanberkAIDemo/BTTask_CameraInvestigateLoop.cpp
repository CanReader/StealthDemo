// © 2025 Canberk. All Rights Reserved.

#include "BTTask_CameraInvestigateLoop.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CameraPawn.h"

UBTTask_CameraInvestigateLoop::UBTTask_CameraInvestigateLoop()
{
	NodeName = "Camera Investigate Loop";
	bNotifyTick = true;
	bCreateNodeInstance = true;

	// Default blackboard key names
	InvestigateOriginKey.SelectedKeyName = FName("InvestigateOrigin");
	InvestigateCountKey.SelectedKeyName = FName("InvestigateCount");
}

EBTNodeResult::Type UBTTask_CameraInvestigateLoop::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CurrentPointIndex = 0;
	CurrentLookTimer = 0.f;
	bLookTargetSet = false;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());
	if (!Camera) return EBTNodeResult::Failed;

	Camera->PausePatrol();

	return EBTNodeResult::InProgress;
}

void UBTTask_CameraInvestigateLoop::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());
	if (!Camera)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if we've completed all investigation points
	if (CurrentPointIndex >= InvestigatePointCount)
	{
		BB->SetValueAsInt(InvestigateCountKey.SelectedKeyName, CurrentPointIndex);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Generate a new random point if we haven't set one yet
	if (!bLookTargetSet)
	{
		FVector Origin = BB->GetValueAsVector(InvestigateOriginKey.SelectedKeyName);

		// Generate a random point around the origin
		FVector RandomOffset = FMath::VRand() * FMath::FRandRange(InvestigateRadius * 0.3f, InvestigateRadius);
		RandomOffset.Z = 0.f; // Keep it on the horizontal plane
		FVector LookPoint = Origin + RandomOffset;

		Camera->SetLookTarget(LookPoint);
		bLookTargetSet = true;
		CurrentLookTimer = 0.f;
	}

	// Count time looking at this point
	CurrentLookTimer += DeltaSeconds;

	if (CurrentLookTimer >= LookDuration)
	{
		CurrentPointIndex++;
		bLookTargetSet = false;
		BB->SetValueAsInt(InvestigateCountKey.SelectedKeyName, CurrentPointIndex);
	}
}

FString UBTTask_CameraInvestigateLoop::GetStaticDescription() const
{
	return FString::Printf(TEXT("Investigate %d points for %.1fs each (radius: %.0f)"),
		InvestigatePointCount, LookDuration, InvestigateRadius);
}
