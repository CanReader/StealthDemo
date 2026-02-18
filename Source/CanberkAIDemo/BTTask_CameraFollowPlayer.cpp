// © 2025 Canberk. All Rights Reserved.

#include "BTTask_CameraFollowPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CameraPawn.h"

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

	// Get the player actor from the blackboard
	AActor* PlayerActor = Cast<AActor>(BB->GetValueAsObject(FName("Character")));
	if (!PlayerActor) return EBTNodeResult::Failed;

	Camera->PausePatrol();
	Camera->SetFollowTarget(PlayerActor);

	return EBTNodeResult::InProgress;
}

void UBTTask_CameraFollowPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	bool bCanSee = BB->GetValueAsBool(FName("bDidSee"));

	// Keep ticking while we can see the player; finish when sight is lost
	if (!bCanSee)
	{
		AAIController* AIController = OwnerComp.GetAIOwner();
		if (AIController)
		{
			if (ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn()))
			{
				Camera->ClearFollowTarget();
			}
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
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
	return TEXT("Follow player until sight is lost");
}
