#include "BaseAIController.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Debugger.h"

DEFINE_LOG_CATEGORY(LOGAI);

ABaseAIController::ABaseAIController()
{
    Tree = CreateDefaultSubobject<UBehaviorTree>(TEXT("Behavior Tree"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Set the default configuration for Perception (They will be changed in subclasses!)
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Sense"));
    SightConfig->SightRadius = 1800.0f;
    SightConfig->LoseSightRadius = 1900.0f;
    SightConfig->PeripheralVisionAngleDegrees = 30.0f;
    SightConfig->SetMaxAge(10);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    HearConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hear Sense"));
    HearConfig->HearingRange = 600.0f;
    HearConfig->SetMaxAge(5);
    HearConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->ConfigureSense(*HearConfig);
    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ABaseAIController::BeginPlay()
{
    Super::BeginPlay();
    if (Tree)
    {
        RunBehaviorTree(Tree);
        Blackboard->InitializeBlackboard(*Tree->BlackboardAsset);
    }

    if (PerceptionComponent)
    {
        PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnPerceptionUpdated);
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetPerceptionUpdated);
        PerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ABaseAIController::OnTargetPerceptionForgotten);
    }
}

void ABaseAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if(Blackboard && PerceptedActor)
    if(Blackboard->GetValueAsBool(FName("bDidSee")))
        Blackboard->SetValueAsVector(FName("LastSeenLocation"),PerceptedActor->GetActorLocation());
}

void ABaseAIController::OnNoticedPlayer(APawn* Player)
{
    msg("I noticed youuuu!");
    Blackboard->SetValueAsBool(FName("bDidSee"), true);
    SetAwarnessState(EPlayerAwarenessState::Suspicious);
}

void ABaseAIController::OnDetectPlayer()
{
    SetAwarnessState(EPlayerAwarenessState::AlarmTriggered);
}

void ABaseAIController::OnHeardPlayer(FVector Location)
{
    Blackboard->SetValueAsBool(FName("bDidHear"), true);
    Blackboard->SetValueAsVector(FName("HeardLocation"), Location);
    SetAwarnessState(EPlayerAwarenessState::Suspicious);
}

void ABaseAIController::OnLostPlayer(bool bIsSightLost)
{
    if(bIsSightLost)
    {
        Blackboard->SetValueAsBool(FName("bDidSee"), false);
    }
    else {
        Blackboard->SetValueAsBool(FName("bDidHear"), false);
    }
    
    SetAwarnessState(EPlayerAwarenessState::LostTarget);
}

void ABaseAIController::SetBlackboardValue(FName Key, UObject* value)
{
    Blackboard->SetValueAsObject(Key, value);
}

void ABaseAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    UE_LOG(LOGAI, Display, TEXT("Perception updated with %d actors"), UpdatedActors.Num());
}

void ABaseAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    UE_LOG(LOGAI, Warning, TEXT("Perception updated for %s with %s at %s"), *Actor->GetName(), *Stimulus.Type.Name.ToString(), *Stimulus.StimulusLocation.ToString());

    const bool bIsPlayer = (Actor == GetWorld()->GetFirstPlayerController()->GetPawn());

    if(Stimulus.WasSuccessfullySensed()){
        Blackboard->SetValueAsObject(FName("Character"),Actor);
        PerceptedActor = Actor;
    }
    else{
        Blackboard->SetValueAsVector(FName("LastSeenLocation"),Stimulus.StimulusLocation);
        Blackboard->ClearValue(FName("Character"));
        PerceptedActor = nullptr;
    }

    // SIGHT
    if (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Newly seen / still seen
            if (bIsPlayer)
                OnNoticedPlayer(Cast<APawn>(Actor));
        }
        else
        {
            // LOST SIGHT
            if (bIsPlayer)
                OnLostPlayer(true);
        }
    }

    // HEARING
    else if (Stimulus.Type == UAISense::GetSenseID(UAISense_Hearing::StaticClass()))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            OnHeardPlayer(Stimulus.StimulusLocation);
        }
        else
        {
            OnLostPlayer(false);
        }
    }
}


void ABaseAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
    msg("I've already forgetten about it");
}
