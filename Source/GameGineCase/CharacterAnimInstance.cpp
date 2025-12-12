// © 2024 Gamegine. All Rights Reserved.


#include "CharacterAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "AgentCharacter.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Character = Cast<ABaseCharacter>(TryGetPawnOwner());
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(Character == nullptr)
	Character = Cast<ABaseCharacter>(TryGetPawnOwner());

	if (Character)
	{
		Velocity = Character->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
		bIsInAir = Character->GetCharacterMovement()->IsFalling();
		bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

		bWeaponEquipped = Character->GetCombatComponent() != nullptr && Character->GetCombatComponent()->GetIsWeaponEquipped();
		bAiming = Character->GetIsAiming();
		bIsCrouched = Character->bIsCrouched;

		FRotator AimRotation = Character->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
		DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
		YawOffset = DeltaRotation.Yaw;
		Direction = CalculateDirection(Velocity,Character->GetActorRotation());

		CharacterRotationLastFrame = CharacterRotation;
		CharacterRotation = Character->GetActorRotation();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
		const float Target = Delta.Yaw / DeltaTime;
		const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
		Lean = FMath::Clamp(Interp, -90.f, 90.f);

		AO_Yaw = Character->GetAO_Yaw();
		AO_Pitch = Character->GetAO_Pitch();
		//UE_LOG(LogTemp, Warning, TEXT("Velocity: %s || Speed: %f"), *Velocity.ToString(),Speed);
		//UE_LOG(LogTemp, Warning, TEXT("AimRotation: %s, || MovementRotation: %s, || DeltaRot: %s"), *AimRotation.ToString(), *MovementRotation.ToString(), *DeltaRot.ToString());
		//UE_LOG(LogTemp, Warning, TEXT("Direction: %.2f"));

		if(Character->GetIsAOLoggingEnabled())
			UE_LOG(LogTemp,Warning,TEXT("Yaw:%.2f Pitch:%.2f Dir: %.2f"),AO_Yaw,AO_Pitch);

		EquippedType = Character->GetEquippedWeaponType();
	}
}
