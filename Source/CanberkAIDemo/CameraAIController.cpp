#include "CameraAIController.h"
#include "CameraPawn.h"
#include "AlertWidget.h"
#include "CharacterHUD.h"
#include "MissionGameMode.h"
#include "Debugger.h"
#include "Kismet/GameplayStatics.h"

ACameraAIController::ACameraAIController()
{
	auto TreeClass = ConstructorHelpers::FObjectFinder<UBehaviorTree>(TEXT("/Game/AI/BT_Camera.BT_Camera"));

	if (TreeClass.Succeeded())
	{
		Tree = TreeClass.Object;
	}
	else
		msg("Failed to find Behavior Tree");

	HearConfig->HearingRange = 800.0f;
	HearConfig->SetMaxAge(HeardForgetTime);
	HearConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearConfig->DetectionByAffiliation.bDetectNeutrals = true;

	SightConfig->SightRadius = 20000.0f;
	SightConfig->LoseSightRadius = 22000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 20.0f;
	SightConfig->SetMaxAge(SightForgetTime);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComponent->ConfigureSense(*HearConfig);
	PerceptionComponent->ConfigureSense(*SightConfig);
}

void ACameraAIController::BeginPlay()
{
	Super::BeginPlay();

	camera = GetPawn<ACameraPawn>();

	SetAwarnessState(EPlayerAwarenessState::Unaware);

	// Register with the player's HUD for screen-space indicators
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ACharacterHUD* HUD = Cast<ACharacterHUD>(PC->GetHUD()))
		{
			HUD->RegisterCamera(this);
		}
	}
}

void ACameraAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ACharacterHUD* HUD = Cast<ACharacterHUD>(PC->GetHUD()))
		{
			HUD->UnregisterCamera(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACameraAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep updating sight escalation every frame
	UpdateSightEscalation(DeltaTime);
}

void ACameraAIController::SetAwarnessState(EPlayerAwarenessState state)
{
	Super::SetAwarnessState(state);

	if (camera)
	{
		switch (state)
		{
		case EPlayerAwarenessState::Alerted:
			camera->ChangeLightColor(FColor::FromHex("A70004FF")); // Red
			camera->SetLightIntensityMode(6000.f, 2000.f, 3.0f); // Aggressive flash
			break;
		case EPlayerAwarenessState::Investigating:
		case EPlayerAwarenessState::Suspicious:
			camera->ChangeLightColor(FColor::FromHex("FFED29FF")); // Yellow
			camera->SetLightIntensityMode(6000.f, 0.f, 0.f); // Steady bright
			break;
		case EPlayerAwarenessState::LostTarget:
			camera->ChangeLightColor(FColor::FromHex("FFA500FF")); // Orange
			camera->SetLightIntensityMode(6000.f, 0.f, 0.f); // Steady bright
			break;
		default:
			camera->ChangeLightColor(FColor::FromHex("82E600FF")); // Green
			camera->SetLightIntensityMode(5000.f, 500.f, 0.5f); // Gentle breathing
			break;
		}
	}
}

void ACameraAIController::OnNoticedPlayer(APawn* Player)
{
	if (bAlertTriggered) return;

	Blackboard->SetValueAsBool(FName("bDidSee"), true);
	Blackboard->SetValueAsObject(FName("Character"), Player);

	// If we see the player while already investigating a noise → immediate alert
	if (CurrentState == EPlayerAwarenessState::Investigating)
	{
		TriggerAlert();
		return;
	}

	// Otherwise go Suspicious and start following
	SetAwarnessState(EPlayerAwarenessState::Suspicious);

	if (camera)
	{
		camera->PlayNoticeSound();
		camera->RotateSpeed = camera->ReactRotateSpeed;
		camera->PausePatrol();
		camera->SetFollowTarget(Player);
	}
}

void ACameraAIController::OnLostPlayer(bool bIsSightLost)
{
	if (bAlertTriggered) return;

	if (bIsSightLost)
	{
		Blackboard->SetValueAsBool(FName("bDidSee"), false);

		if (camera)
		{
			camera->ClearFollowTarget();
		}

		if (CurrentState == EPlayerAwarenessState::Suspicious)
		{
			if (camera)
			{
				camera->RotateSpeed = camera->ReactRotateSpeed;
			}

			SetAwarnessState(EPlayerAwarenessState::Investigating);
		}
	}
	else
	{
		// Lost hearing
		Blackboard->SetValueAsBool(FName("bDidHear"), false);
		SetAwarnessState(EPlayerAwarenessState::LostTarget);
	}
}

void ACameraAIController::OnDetectPlayer()
{
	TriggerAlert();
}

void ACameraAIController::OnHeardPlayer(FVector Location)
{
	if (bAlertTriggered) return;

	// Don't downgrade if we're already in a more important state
	if (CurrentState == EPlayerAwarenessState::Suspicious || CurrentState == EPlayerAwarenessState::Alerted)
		return;

	Blackboard->SetValueAsBool(FName("bDidHear"), false);
	Blackboard->SetValueAsBool(FName("bDidHear"), true);
	Blackboard->SetValueAsVector(FName("HeardLocation"), Location);
	Blackboard->SetValueAsVector(FName("InvestigateOrigin"), Location);
	Blackboard->SetValueAsInt(FName("InvestigateCount"), 0);

	if (camera)
	{
		camera->RotateSpeed = camera->ReactRotateSpeed;
		camera->PausePatrol();
	}

	SetAwarnessState(EPlayerAwarenessState::Investigating);
}

void ACameraAIController::UpdateSightEscalation(float DeltaTime)
{
	if (bAlertTriggered) return;

	bool bCanSeePlayer = Blackboard && Blackboard->GetValueAsBool(FName("bDidSee"));

	if (bCanSeePlayer && CurrentState != EPlayerAwarenessState::Alerted)
	{
		ContinuousSightTime += DeltaTime;

		// Grace period: first 0.5s of sight doesn't count
		if (ContinuousSightTime > SightGracePeriod)
		{
			float Progress = FMath::Clamp(SightAccumulatedTime / SightAlertThreshold, 0.f, 1.f);

			// Accelerate accumulation past 60%
			float Rate = Progress > 0.6f ? SightAccelerationRate : 1.0f;
			SightAccumulatedTime += DeltaTime * Rate;

			// Check threshold
			if (SightAccumulatedTime >= SightAlertThreshold)
			{
				TriggerAlert();
			}
		}
	}
	else if (!bCanSeePlayer)
	{
		// Reset grace period on sight loss
		ContinuousSightTime = 0.f;

		if (SightAccumulatedTime > 0.f)
		{
			// Decay the timer when not seeing the player
			SightAccumulatedTime = FMath::Max(0.f, SightAccumulatedTime - SightDecayRate * DeltaTime);
		}
	}
}

void ACameraAIController::TriggerAlert()
{
	if (bAlertTriggered) return;
	bAlertTriggered = true;

	SetAwarnessState(EPlayerAwarenessState::Alerted);

	if (camera)
	{
		camera->PlayAlertSound();
		camera->PausePatrol();

		// Lock onto the player during alert window
		if (AActor* Player = Cast<AActor>(Blackboard->GetValueAsObject(FName("Character"))))
		{
			camera->SetFollowTarget(Player);
			camera->RotateSpeed = camera->ReactRotateSpeed * 1.5f;
		}
	}

	// Restart mission after 1.5 second delay
	GetWorldTimerManager().SetTimer(
		AlertRestartTimerHandle,
		this,
		&ACameraAIController::OnAlertDelayFinished,
		1.5f,
		false
	);
}

void ACameraAIController::OnAlertDelayFinished()
{
	if (AMissionGameMode* GameMode = Cast<AMissionGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GameMode->RestartMission();
	}
}

void ACameraAIController::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (ACameraPawn* Camera = Cast<ACameraPawn>(GetPawn()))
	{
		OutLocation = Camera->GetHeadLocation();
		OutRotation = Camera->GetHeadRotation();
	}
	else
	{
		Super::GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}
