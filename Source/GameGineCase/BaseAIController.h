#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include <Perception/AISenseConfig_Sight.h>
#include <Perception/AISenseConfig_Hearing.h>
#include "BaseAIController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LOGAI, Display, All);

UENUM(BlueprintType)
enum class EPlayerAwarenessState : uint8
{
    Unaware UMETA(DisplayName = "Unaware"), // 0
    Suspicious UMETA(DisplayName = "Suspicious"), // 1
    Investigating UMETA(DisplayName = "Investigating"), // 2
    Alerted UMETA(DisplayName = "Alerted"), // 3
    Engaging UMETA(DisplayName = "Engaging"), // 4
    LostTarget UMETA(DisplayName = "Lost Target"), // 5
    Searching UMETA(DisplayName = "Searching"), // 6
    ReturningToPatrol UMETA(DisplayName = "Returning to Patrol"), // 7
    AlarmTriggered UMETA(DisplayName = "Alarm Triggered") // 8
};

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API ABaseAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABaseAIController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    EPlayerAwarenessState GetAwarnessState() { return CurrentState; }
    UFUNCTION(BlueprintCallable)
    void SetAwarnessState(EPlayerAwarenessState state) { CurrentState = state; Blackboard->SetValueAsInt(FName("Awarness"),static_cast<int>(state));}
    UFUNCTION(BlueprintCallable)
    void SetBlackboardValue(FName Key, UObject* value);

protected:
    UFUNCTION()
    virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
    UFUNCTION()
    virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
    UFUNCTION()
    virtual void OnTargetPerceptionForgotten(AActor* Actor);

    virtual void OnNoticedPlayer(APawn* Player);
    virtual void OnDetectPlayer();
    virtual void OnHeardPlayer(FVector Location);
    virtual void OnLostPlayer(bool bIsSightLost);

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SightForgetTime = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float HeardForgetTime = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* Tree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    EPlayerAwarenessState CurrentState;

    AActor* PerceptedActor;

    UAISenseConfig_Sight* SightConfig;
    UAISenseConfig_Hearing* HearConfig;

public:
    FORCEINLINE UAISenseConfig_Sight* GetSightConfig() { return SightConfig; }
    FORCEINLINE UAISenseConfig_Hearing* GetHearConfig() { return HearConfig; }

    FORCEINLINE void SetSightConfig(UAISenseConfig_Sight* config){SightConfig = config; }

    //FORCEINLINE APawn* GetPerceptedPawn() { return Cast<APawn*>(PerceptedActor); }
};
