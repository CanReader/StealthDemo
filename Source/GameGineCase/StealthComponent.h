// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StealthComponent.generated.h"

class ABaseCharacter;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEGINECASE_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStealthComponent();
	friend class ABaseCharacter;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void OnCoverOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetCharacter(ABaseCharacter*);

	void Attach();
	void Detach();

	void Crouch();
	void UnCrouch();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* CrouchAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* UncrouchAnimMontage;

	//A helper function for playing crouch and cover montages
	void PlayMontage(UAnimMontage* Montage, TFunction<void()> BlendOutFunction);

	//a helper function to debug interception of character
	TArray<FHitResult> PerformCapsuleTraceForObjects(UWorld*, FVector,FVector,float,float, TArray<TEnumAsByte<EObjectTypeQuery>>, bool);

	//Needed variables to access for crouching and cover mechanics
	class ABaseCharacter* Character;
	class UCapsuleComponent* Capsule;
	class USkeletalMeshComponent* Mesh;
	class UAnimInstance* AnimInstance;
	
#pragma region Cover System Variables
	bool bWallDetected;
	bool bCanCover;
	FVector WallNormal;
	FVector	WallTraceEnd;

#pragma endregion

	FVector DefaultCameraArmLocation;
	FVector CrouchedCameraArmLocation;

	FVector DefaultMeshLocation;
	FVector CrouchedMeshLocation;

	bool bCanCrouch;
	bool bIsCrouching;
	float CrouchInterpSpeed = 10.f;

public:
	FORCEINLINE bool GetIsCrouching() {
		return bIsCrouching;
	}

	FORCEINLINE bool GetCanCovering() {
		return bCanCover;
	}

	FORCEINLINE bool GetIsCovering() {
		return bCanCover && bWallDetected;
	}

	//This get function cannot be inline due to Character header included in source file
	bool GetCanCrouch();
};
