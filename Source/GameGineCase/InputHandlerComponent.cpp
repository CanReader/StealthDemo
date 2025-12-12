// © 2024 Gamegine. All Rights Reserved.


#include "InputHandlerComponent.h"
#include <EnhancedInputSubsystems.h>
#include <EnhancedInputComponent.h>
#include "AgentCharacter.h"

UInputHandlerComponent::UInputHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UInputHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	Character = Cast<AAgentCharacter>(GetOwner());

	if (Character)
	{
		APlayerController* LocalController = Cast<APlayerController>(Character->GetInstigatorController());

		if (LocalController) {
			UEnhancedInputLocalPlayerSubsystem* InputHandler = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalController->GetLocalPlayer());

			if (InputHandler)
				InputHandler->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	else
	{
		UE_LOG(LogMainCharacter, Error, TEXT("Character could not get!"));
	}
}


void UInputHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UInputHandlerComponent::BindInputActions(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnchancedComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	Character = Cast<AAgentCharacter>(GetOwner());

	if (Character && EnchancedComponent) {

		EnchancedComponent->BindAction(JumpAction, ETriggerEvent::Started, Character, &ACharacter::Jump);
		EnchancedComponent->BindAction(JumpAction, ETriggerEvent::Completed, Character, &ACharacter::StopJumping);
		EnchancedComponent->BindAction(MoveAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::MoveControl);
		EnchancedComponent->BindAction(LookAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::LookControl);
		EnchancedComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::CrouchAction);
		EnchancedComponent->BindAction(SprintAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::SprintAction);
		EnchancedComponent->BindAction(CoverAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::CoverAction);
		EnchancedComponent->BindAction(EquipAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::EquipAction);
		EnchancedComponent->BindAction(FireAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::FireAction);
		EnchancedComponent->BindAction(AimAction, ETriggerEvent::Triggered, Character, &AAgentCharacter::AimAction);
		
	}
	else
	{
		if (Character == nullptr) {
			UE_LOG(LogMainCharacter, Error, TEXT("Character has not been found!"));
		}
		else {
			UE_LOG(LogMainCharacter, Error, TEXT("Input Component has not been found!"));
		}
	}
}

