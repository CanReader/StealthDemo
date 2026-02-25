// © 2025-2026 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CameraInvestigateLoop.generated.h"

/**
 * Latent BT task: generates 3 random points around InvestigateOrigin,
 * looks at each for 3 seconds, then returns Success.
 */
UCLASS()
class CANBERKAIDEMO_API UBTTask_CameraInvestigateLoop : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_CameraInvestigateLoop();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

private:
	/** How many points to investigate */
	UPROPERTY(EditAnywhere, Category = "Investigation")
	int32 InvestigatePointCount = 3;

	/** How long to look at each point (seconds) */
	UPROPERTY(EditAnywhere, Category = "Investigation")
	float LookDuration = 3.0f;

	/** Radius around origin to generate random look points */
	UPROPERTY(EditAnywhere, Category = "Investigation")
	float InvestigateRadius = 500.0f;

	/** Blackboard key for the investigation origin */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InvestigateOriginKey;

	/** Blackboard key for the investigation count tracker */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InvestigateCountKey;

	/** Random variance applied to look duration per point (seconds) */
	UPROPERTY(EditAnywhere, Category = "Investigation")
	float LookDurationVariance = 0.5f;

	/** Current point index during investigation */
	int32 CurrentPointIndex = 0;

	/** Timer for current look duration */
	float CurrentLookTimer = 0.f;

	/** Actual duration for the current point (LookDuration +/- variance) */
	float CurrentPointDuration = 0.f;

	/** Whether we've set the look target for the current point */
	bool bLookTargetSet = false;
};
