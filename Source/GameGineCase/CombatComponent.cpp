// © 2024 Gamegine. All Rights Reserved.

#include "CombatComponent.h"
#include "AgentCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CarriedAmmo.Add(EWeaponType::EWT_Pistol,80);
	CarriedAmmo.Add(EWeaponType::EWT_Rifle,160);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Character = Cast<AAgentCharacter>(GetOwner());
	if (Character)
		DefaultFOV = Character->GetCamera()->FieldOfView;
	else
		DefaultFOV = 90;
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	InterpFOV(DeltaTime);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character)
	{
		Character->GetCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::AttachWeapon(AWeapon* Weapon)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || Weapon == nullptr || EquippedWeapon == nullptr) return;

	FName SocketName = Weapon->GetType() == EWeaponType::EWT_Rifle ? FName("RightHandSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
		HandSocket->AttachActor(Weapon, Character->GetMesh());
}

void UCombatComponent::StartAim(bool pressed)
{
	if (!pressed)
	{
		bIsAiming = false;
		Character->ChangeSpeed(Character->BaseAimSpeed);
	}
	else {
		bIsAiming = true;
		Character->ChangeSpeed(Character->BaseWalkSpeed);
	}
}


void UCombatComponent::EquipWeapon(AWeapon* weapon)
{
	if (Character == nullptr)
		Character = Cast<AAgentCharacter>(GetOwner());
	if (weapon != nullptr) {
	EquippedWeapon = weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	EquippedWeapon->SetOwner(GetOwner());
	EquippedWeapon->ShowPickupWidget(false);
	AttachWeapon(weapon);
	auto hud = Character->GetHUD();
	UGameplayStatics::PlaySoundAtLocation(GetWorld(),weapon->GetEquipSound(), Character->GetActorLocation());
	
	if(Character->bIsCrouched)
		Character->MakeNoise(1.2,Character,Character->GetActorLocation());
	else
		Character->MakeNoise(0.6,Character,Character->GetActorLocation());
	
	Reload();

	Character->bUseControllerRotationYaw = true;

	UE_LOG(LogTemp,Warning, TEXT("%s Equipped %s"), *GetOwner()->GetName(), *weapon->GetName());
	}

}

void UCombatComponent::Reload()
{
	if (EquippedWeapon)
	{
		Character->PlayReloadMontage(EquippedWeapon->GetType());
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), EquippedWeapon->ReloadSound, Character->GetActorLocation());
		int32 EmptyAmount = EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetAmmo();

		if (CarriedAmmo.Contains(EquippedWeapon->GetType()))
		{
			int32 Carried = CarriedAmmo[EquippedWeapon->GetType()];
			int32 ReloadAmount = FMath::Clamp(
				EmptyAmount,
				0,
				FMath::Min(EmptyAmount,Carried));

			CarriedAmmo[EquippedWeapon->GetType()] -= ReloadAmount;
			EquippedWeapon->AddAmmo(ReloadAmount);

			Character->GetHUD()->UpdateAmmo(EquippedWeapon->GetAmmo(),CarriedAmmo[EquippedWeapon->GetType()]);
		}
	}
}

void UCombatComponent::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{

}

