// © 2024 Gamegine. All Rights Reserved.

#include "StealthComponent.h"
#include "AgentCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AgentCharacter.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/ArrowComponent.h"
#include "Math/UnrealMathUtility.h"


UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultCameraArmLocation = Character->GetCameraArm()->GetRelativeLocation();
	CrouchedCameraArmLocation = DefaultCameraArmLocation;
}

void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character == nullptr) {
		UE_LOG(LogTemp,Warning,TEXT("Character is nullptr!"))
		return;
	}

		FVector StartLocation = Character->GetCoverArrowComponent()->GetComponentLocation();
		StartLocation.Z += 30;

		auto types = TArray<TEnumAsByte<EObjectTypeQuery>>();
		types.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

		auto result = PerformCapsuleTraceForObjects(
			GetWorld(),
			StartLocation,
			StartLocation,
			58,0.5, types, false);

		bWallDetected = result.Num() > 0;

		if (GetIsCovering())
		{
			WallNormal = result[0].Normal;
			WallTraceEnd = result[0].TraceEnd;
			FRotator WallRot = FRotationMatrix::MakeFromX(WallNormal).Rotator();
			float NewCharYaw = WallRot.Yaw + 180;
			Character->SetActorRotation(FRotator(0,NewCharYaw,0));
			
		}
}


void UStealthComponent::Attach()
{
	if (!bWallDetected)
		return;

	bCanCover = true;
	if (false && GetIsCovering()) //TODO: Fix
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		Character->GetCameraArm()->bUsePawnControlRotation = true;
		Character->ChangeSpeed(Character->BaseCoverSpeed);
	}
}

void UStealthComponent::Detach()
{
	bCanCover = false;

	if (false && GetIsCovering())
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		Character->GetCameraArm()->bUsePawnControlRotation = true;
		Character->ChangeSpeed(Character->BaseWalkSpeed);
	}
}


TArray<FHitResult> UStealthComponent::PerformCapsuleTraceForObjects(UWorld* World, FVector StartLocation, FVector EndLocation, float Radius, float HalfHeight, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes, bool bDrawDebug)
{
	TArray<FHitResult> HitResults;
	FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character); // Ignore the owner actor

	World->SweepMultiByObjectType(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ObjectTypes),
		Shape,
		Params
	);

	if (bDrawDebug){
		if (HitResults.Num() > 0)
		{
			DrawDebugCapsule(World, StartLocation, HalfHeight, Radius, FQuat::Identity, FColor::Green, false, 0.01f);
		}
		else{
			DrawDebugCapsule(World, EndLocation, HalfHeight, Radius, FQuat::Identity, FColor::Red, false, 0.01f);
		}
	}

	return HitResults;
}


void UStealthComponent::SetCharacter(ABaseCharacter* character)
{
		Character = character;
		Capsule = Character->GetCapsuleComponent();
		Mesh = Character->GetMesh();
}

void UStealthComponent::OnCoverOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, "Colliding");
}

void UStealthComponent::Crouch()
{
	if (GetCanCrouch()) {
		//Character->GetCharacterMovement()->DisableMovement();

		if (CrouchAnimMontage)
			PlayMontage(CrouchAnimMontage, [this]() {Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);});
			
		Character->Crouch();
		Character->ChangeSpeed(Character->BaseCrouchSpeed);
		Character->bIsCrouched = true;
		auto CamLoc = Character->GetCameraArm()->GetRelativeLocation();
		CamLoc.X -= 40;
		CamLoc.Z -= Character->CrouchCameraHeightOffset;
		Character->GetCameraArm()->SetRelativeLocation(CamLoc);
		Character->GetMesh()->AddRelativeLocation(FVector(-40, 0, 0));

	}
}

void UStealthComponent::UnCrouch()
{
	if (GetCanCrouch()) {

		if(UncrouchAnimMontage)
		PlayMontage(UncrouchAnimMontage, [this]() {Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);});

		Character->UnCrouch();
		Capsule->SetCapsuleHalfHeight(Character->DefaultCapsuleHalfHeight);
		Character->ChangeSpeed(Character->BaseWalkSpeed);
		Character->bIsCrouched = false;
		auto CamLoc = Character->GetCameraArm()->GetRelativeLocation();
		CamLoc.X += 40;
		CamLoc.Z += Character->CrouchCameraHeightOffset;
		Capsule->SetCapsuleRadius(25);
		Character->GetCameraArm()->SetRelativeLocation(CamLoc);
		Character->GetMesh()->AddRelativeLocation(FVector(+40, 0, 0));

	}
}

void UStealthComponent::PlayMontage(UAnimMontage* Montage, TFunction<void()> BlendOutFunction)
{
	if (Montage)
	{
		AnimInstance = Mesh->GetAnimInstance();
		if (AnimInstance)
		{

			AnimInstance->Montage_Play(Montage);
			
			if (BlendOutFunction) {
				FOnMontageBlendingOutStarted BlendingOutDelegate;
				BlendingOutDelegate.BindLambda([BlendOutFunction](UAnimMontage* Montage, bool bInterrupted)
					{
						BlendOutFunction();
					});
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
			}
		}
	}
}

bool UStealthComponent::GetCanCrouch()
{
	return Character && Character->GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking;
}

