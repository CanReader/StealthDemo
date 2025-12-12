// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "CameraAIController.h"
#include "CameraPawn.generated.h"

UCLASS()
class GAMEGINECASE_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACameraPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void MoveCameraDelta(FVector TargetLocation, float dt);
	UFUNCTION(BlueprintCallable)
	void MoveCamera(FVector TargetLocation);
	UFUNCTION(BlueprintCallable)
	void MoveCameraToActor(AActor* TargetActor);

  FTransform GetScannerViewTransform() const;
	FVector GetHeadLocation();
	FRotator GetHeadRotation();

	void ChangeLightColor(FColor color);

private:
	void CalculatePatrolPoints();
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

#pragma region Setting patrol points
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Patrol, meta = (AllowPrivateAccess = "true"))
	float RotateSpeed = 0.8; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta= (AllowPrivateAccess = true))
	TArray<FVector> PatrolPoints;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta= (AllowPrivateAccess = true))
	FVector TargetPoint;
	
	FVector CurrentPoint;
#pragma endregion
};
