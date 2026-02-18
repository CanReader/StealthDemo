// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CameraFollowPlayer.generated.h"

UENUM()
enum class ECameraFollowPhase : uint8
{
	Following,
	LookingAtLastSeen,
	Investigating
};

/**
 * Latent BT task: follows the player, then investigates when sight is lost.
 * Phase 1: Follow player while visible
 * Phase 2: Look at last seen location for LookAtLastSeenDuration seconds
 * Phase 3: Look at 3 random points around last seen location
 * Returns Success after investigation is complete, or Failed if alert triggers.
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

private:
	void EnterInvestigatePhase(UBehaviorTreeComponent& OwnerComp);

	ECameraFollowPhase CurrentPhase = ECameraFollowPhase::Following;

	UPROPERTY(EditAnywhere, Category = "Camera AI")
	float LookAtLastSeenDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Camera AI")
	float InvestigatePointDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Camera AI")
	float InvestigateRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Camera AI")
	int32 InvestigatePointCount = 3;

	UPROPERTY(EditAnywhere, Category = "Camera AI")
	float DurationVariance = 0.5f;

	float PhaseTimer = 0.f;
	float CurrentPhaseDuration = 0.f;
	FVector LastSeenLocation;
	TArray<FVector> InvestigatePoints;
	int32 CurrentInvestigateIndex = 0;
};
