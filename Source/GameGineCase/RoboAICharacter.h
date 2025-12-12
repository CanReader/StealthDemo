// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.h"
#include "RoboAICharacter.generated.h"

UCLASS()
class GAMEGINECASE_API ARoboAICharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ARoboAICharacter();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	TObjectPtr<AWeapon> Weapon;

public:

};
