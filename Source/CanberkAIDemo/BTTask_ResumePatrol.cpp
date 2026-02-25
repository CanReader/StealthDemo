// © 2025-2026 Canberk. All Rights Reserved.

#include "BTTask_ResumePatrol.h"
#include "AIController.h"
#include "CameraPawn.h"
#include "CameraAIController.h"

UBTTask_ResumePatrol::UBTTask_ResumePatrol()
{
	NodeName = "Resume Patrol";
}

EBTNodeResult::Type UBTTask_ResumePatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	ACameraPawn* Camera = Cast<ACameraPawn>(AIController->GetPawn());
	if (!Camera) return EBTNodeResult::Failed;

	// Restore normal rotation speed
	if (ACameraAIController* CamAI = Cast<ACameraAIController>(AIController))
	{
		Camera->RotateSpeed = Camera->PatrolRotateSpeed;
	}

	Camera->ResumePatrol();

	return EBTNodeResult::Succeeded;
}

FString UBTTask_ResumePatrol::GetStaticDescription() const
{
	return TEXT("Resume camera patrol movement");
}
