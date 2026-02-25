// © 2025-2026 Canberk. All Rights Reserved.

#include "BTTask_TriggerCameraAlert.h"
#include "AIController.h"
#include "CameraAIController.h"

UBTTask_TriggerCameraAlert::UBTTask_TriggerCameraAlert()
{
	NodeName = "Trigger Camera Alert";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_TriggerCameraAlert::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACameraAIController* CamAI = Cast<ACameraAIController>(OwnerComp.GetAIOwner());
	if (!CamAI) return EBTNodeResult::Failed;

	CamAI->TriggerAlert();

	// Return InProgress — the level will restart via timer, so the tree just stops here
	return EBTNodeResult::InProgress;
}

FString UBTTask_TriggerCameraAlert::GetStaticDescription() const
{
	return TEXT("Trigger alert: red light → restart mission after delay");
}
