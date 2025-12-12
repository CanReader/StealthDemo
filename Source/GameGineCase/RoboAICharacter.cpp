// © 2024 Gamegine. All Rights Reserved.

#include "RoboAICharacter.h"

ARoboAICharacter::ARoboAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsAI = true;

}

void ARoboAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARoboAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

