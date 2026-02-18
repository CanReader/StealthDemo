// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResumePatrol.generated.h"

/**
 * Simple BT task: calls ResumePatrol() on the CameraPawn and finishes immediately.
 */
UCLASS()
class CANBERKAIDEMO_API UBTTask_ResumePatrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ResumePatrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
