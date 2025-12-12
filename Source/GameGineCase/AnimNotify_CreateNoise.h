// © 2024 Gamegine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "AnimNotify_CreateNoise.generated.h"

/**
 * 
 */
UCLASS()
class GAMEGINECASE_API UAnimNotify_CreateNoise : public UAnimNotify_PlaySound
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ToolTip = "Loudness of the generated noise (0.0-1.0)"))
	float Loudness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ToolTip = "Maximum distance at which noise can be heard"))
	float MaxRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ToolTip = "Place tag to distinguish notifies"))
	FName Tag = FName("None");

};
