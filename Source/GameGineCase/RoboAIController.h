#pragma once

#include "CoreMinimal.h"
#include "BaseAIController.h"
#include "RoboAIController.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API ARoboAIController : public ABaseAIController
{
    GENERATED_BODY()

public:
    ARoboAIController();

protected:
    void OnNoticedPlayer(APawn* Player) override;
    void OnDetectPlayer() override;
    void OnHeardPlayer(FVector Location) override;
};
