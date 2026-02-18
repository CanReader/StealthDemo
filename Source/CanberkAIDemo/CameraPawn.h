// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "CameraPawn.generated.h"

class ACameraAIController;
class UAlertWidget;

UCLASS()
class CANBERKAIDEMO_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	FTransform GetScannerViewTransform() const;
	FVector GetHeadLocation();
	FRotator GetHeadRotation();

	void ChangeLightColor(FColor color);

	/** Set intensity pulse parameters — called by AI controller on state change */
	void SetLightIntensityMode(float InBaseIntensity, float InPulseAmplitude, float InPulseFrequency);

#pragma region Smooth Movement API
	/** Set a world-space point the camera should look at (used by BT tasks) */
	void SetLookTarget(FVector InTarget);

	/** Follow a specific actor each frame (e.g. the player) */
	void SetFollowTarget(AActor* InActor);
	void ClearFollowTarget();

	/** Patrol control — BT tasks call these */
	void ResumePatrol();
	void PausePatrol();

	FORCEINLINE bool IsPatrolling() const { return bIsPatrolling; }
#pragma endregion

	/** Play one-shot sound cues (called by AI controller) */
	void PlayNoticeSound();
	void PlayAlertSound();
	void PlayHearingNoticeSound();

	/** Escalation sound — rising pitch tone while detection meter fills */
	void StartEscalationSound();
	void StopEscalationSound();
	void UpdateEscalationPitch(float Progress);

	/** Get the alert widget for progress bar updates */
	UAlertWidget* GetNotifyWidget() const;

	/** Show/hide the overhead widget */
	void SetNotifyWidgetVisible(bool bVisible);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float RotateSpeed = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float PatrolRotateSpeed = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float ReactRotateSpeed = 4.0f;

private:
	void CalculatePatrolPoints();
	void UpdateBoneRotation(float DeltaTime);
	void UpdatePatrol(float DeltaTime);
	void UpdateLightColor(float DeltaTime);
	void UpdateLightIntensity(float DeltaTime);

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Camera|Sight")
	void SetScannerSight(float NewSightRadius, float NewPeripheralVisionAngleDegrees);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPoseableMeshComponent> mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpotLightComponent> ScannerLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> NotifyWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator CameraLookOffset = FRotator(80,0,90);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> ExplosionEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float Health = 5;

#pragma region Patrol Settings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float PointsOffset = 350;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	int32 DistanceBetweenPoints = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	int32 DistanceFromPawn = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float RotateAngle = -50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	bool bIsPatrolling = true;

	/** How long to pause at each patrol endpoint before reversing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float PatrolWaitTime = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TArray<FVector> PatrolPoints;

	int32 CurrentPatrolIndex = 0;
	float PatrolWaitTimer = 0.f;
	bool bWaitingAtPatrolPoint = false;
#pragma endregion

#pragma region Light Interpolation
	FLinearColor CurrentLightColor;
	FLinearColor TargetLightColor;

	UPROPERTY(EditAnywhere, Category = "Camera|Light")
	float LightColorInterpSpeed = 3.0f;

	float BaseLightIntensity = 5000.f;
	float PulseAmplitude = 500.f;
	float PulseFrequency = 0.5f;
	float PulseTimer = 0.f;
#pragma endregion

#pragma region Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> ServoLoopCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> NoticeSoundCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> AlertSoundCue;

	/** Sound that plays when camera hears a noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> HearingNoticeSoundCue;

	/** Looping tone that plays during sight escalation — pitch rises with progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> EscalationSoundCue;

	/** Min pitch for escalation sound (at 0% progress) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	float EscalationMinPitch = 0.6f;

	/** Max pitch for escalation sound (at 100% progress) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Sound", meta = (AllowPrivateAccess = "true"))
	float EscalationMaxPitch = 2.0f;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ServoAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> EscalationAudioComponent;
#pragma endregion

	/** The world-space point the camera is always smoothly interpolating toward */
	FVector DesiredLookTarget;

	/** Current interpolated bone rotation */
	FRotator CurrentBoneRotation;

	/** Actor to follow each frame (overrides DesiredLookTarget) */
	UPROPERTY()
	AActor* FollowTargetActor = nullptr;
};
