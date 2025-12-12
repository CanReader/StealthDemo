// © 2024 Gamegine. All Rights Reserved.


#include "BaseCharacter.h"
#include "CombatComponent.h"
#include "StealthComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Stealth = CreateDefaultSubobject<UStealthComponent>(TEXT("Stealth"));
	Stealth->SetCharacter(this);
	CoverArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Cover Arrow"));
	CoverArrow->SetupAttachment(GetMesh());
	CoverArrow->SetRelativeLocation(FVector(0, 27, 130));
	CoverArrow->SetRelativeRotation(FRotator(0, 90, 0));
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	TargetSpeed = BaseWalkSpeed;
	
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float NewSpeed = FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, InterpSpeed);
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void ABaseCharacter::EquipWeapon(AWeapon* EquipWeapon)
{
}


