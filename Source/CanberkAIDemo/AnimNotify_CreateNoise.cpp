// AnimNotify_CreateNoise.cpp
#include "AnimNotify_CreateNoise.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Pawn.h"

void UAnimNotify_CreateNoise::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        FVector NoiseLocation = MeshComp->GetComponentLocation();
        
        // Get the instigator correctly
        APawn* InstigatorPawn = Cast<APawn>(MeshComp->GetOwner());
        if (!InstigatorPawn)
        {
            // Get the instigator from the owner actor
            InstigatorPawn = MeshComp->GetOwner()->GetInstigator();
        }

        UAISense_Hearing::ReportNoiseEvent(
            GetWorld(),
            NoiseLocation,
            Loudness,
            InstigatorPawn,  // Can be nullptr
            MaxRange,
            Tag
        );
    }
}