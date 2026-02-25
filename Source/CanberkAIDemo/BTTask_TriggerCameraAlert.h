// © 2025-2026 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TriggerCameraAlert.generated.h"

/**
 * BT task: triggers the camera alert state — red light, then level restart.
 * Returns InProgress and completes when TriggerAlert is done.
 */
UCLASS()
class CANBERKAIDEMO_API UBTTask_TriggerCameraAlert : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TriggerCameraAlert();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
