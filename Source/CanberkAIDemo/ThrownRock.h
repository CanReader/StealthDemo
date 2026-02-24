// © 2025 Canberk. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrownRock.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class CANBERKAIDEMO_API AThrownRock : public AActor
{
	GENERATED_BODY()

public:
	AThrownRock();

	void Launch(FVector Direction, float Speed);

protected:
	UFUNCTION()
	void OnRockHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> RockMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Rock")
	float NoiseLoudness = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rock")
	float SelfDestructDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Rock")
	TObjectPtr<USoundBase> ImpactSound;

	bool bHasImpacted = false;
};
