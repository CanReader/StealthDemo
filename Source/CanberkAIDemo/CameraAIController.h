#pragma once

#include "CoreMinimal.h"
#include "BaseAIController.h"
#include "CameraAIController.generated.h"

UCLASS()
class CANBERKAIDEMO_API ACameraAIController : public ABaseAIController
{
    GENERATED_BODY()
public:
    ACameraAIController();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    void SetAwarnessState(EPlayerAwarenessState state) override;
    void OnLostPlayer(bool bIsSightLost) override;
    void OnNoticedPlayer(APawn* Player) override;
    void OnDetectPlayer() override;
    void OnHeardPlayer(FVector Location) override;

    /** Trigger alert state - red light, then restart mission after delay */
    void TriggerAlert();

    /** Get the accumulated sight time (for progress bar in BT tasks) */
    FORCEINLINE float GetSightProgress() const { return SightAccumulatedTime / SightAlertThreshold; }

protected:
    virtual void GetActorEyesViewPoint(FVector&, FRotator&) const override;

    class ACameraPawn* camera;

private:
    void UpdateSightEscalation(float DeltaTime);
    void OnAlertDelayFinished();

    /** Time the player has been continuously visible */
    float SightAccumulatedTime = 0.f;

    /** Seconds of continuous sight before triggering alert */
    UPROPERTY(EditDefaultsOnly, Category = "Camera AI")
    float SightAlertThreshold = 5.0f;

    /** Decay rate when player is not visible (per second) */
    UPROPERTY(EditDefaultsOnly, Category = "Camera AI")
    float SightDecayRate = 1.5f;

    /** Grace period before sight starts accumulating */
    UPROPERTY(EditDefaultsOnly, Category = "Camera AI")
    float SightGracePeriod = 0.5f;

    /** Current continuous sight time (for grace period tracking) */
    float ContinuousSightTime = 0.f;

    /** Acceleration multiplier after 60% progress */
    UPROPERTY(EditDefaultsOnly, Category = "Camera AI")
    float SightAccelerationRate = 1.8f;

    /** Timer for the delay between alert and level restart */
    FTimerHandle AlertRestartTimerHandle;

    /** Whether alert has already been triggered (prevent double-trigger) */
    bool bAlertTriggered = false;
};
