#pragma once

#include "CoreMinimal.h"
#include "BaseAIController.h"
#include "CameraAIController.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API ACameraAIController : public ABaseAIController
{
    GENERATED_BODY()
public:
    ACameraAIController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void OnNoticedPlayer(APawn* Player) override;
    void OnDetectPlayer() override;
    void OnHeardPlayer(FVector Location) override;

protected:
    virtual void GetActorEyesViewPoint(FVector&, FRotator&) const override;
};
