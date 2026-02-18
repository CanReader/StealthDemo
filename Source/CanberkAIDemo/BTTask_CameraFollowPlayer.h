// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CameraFollowPlayer.generated.h"

/**
 * Latent BT task: sets camera to follow the player.
 * Ticks while bDidSee is true. Returns Success when sight is lost.
 */
UCLASS()
class CANBERKAIDEMO_API UBTTask_CameraFollowPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CameraFollowPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
