// � 2024 Gamegine. All Rights Reserved.

#include "AgentCharacter.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Components/CapsuleComponent.h>
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystems.h>
#include "InputHandlerComponent.h"
#include "DrawDebugHelpers.h" 
#include "Engine/World.h"
#include "Components/ArrowComponent.h"

DEFINE_LOG_CATEGORY(LogMainCharacter);

AAgentCharacter::AAgentCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Setting capsule size for collision detection
	GetCapsuleComponent()->InitCapsuleSize(50.f, DefaultCapsuleHalfHeight);

	//Configure character movement defaults
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 400.0f, 0.0f);
	
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxAcceleration           = 1024.f;   // default is 2048, lower = smoother start
	GetCharacterMovement()->BrakingDecelerationWalking = 1024.f;  // lower = smoother stop
	GetCharacterMovement()->GroundFriction            = 6.0f;     // tune for slide vs stickiness
	GetCharacterMovement()->BrakingFrictionFactor     = 1.0f;     // usually fine

	//Disable controller rotation to align character rotation with movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	InputHandler = CreateDefaultSubobject<UInputHandlerComponent>(TEXT("Input Handler"));
	
	//Initialization of Spring Arm
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->SetRelativeLocation(FVector(10, 10, 80));
	CameraArm->TargetArmLength = 80.0f;
	CameraArm->bUsePawnControlRotation = true;
  	CameraArm->bDoCollisionTest = true;
  	CameraArm->ProbeSize        = 40.f;         // try 12–16
  	CameraArm->ProbeChannel     = ECC_Camera;


	//Initialization of Third Person Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->SetRelativeLocation(FVector(0, 30, 0));
	Camera->bUsePawnControlRotation = false;

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    // Set default standing height
    GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultCapsuleHalfHeight);

    // Tell movement what the crouched half-height should be
    GetCharacterMovement()->CrouchedHalfHeight = CrouchCapsuleHalfHeight;
	CrouchCameraHeightOffset = DefaultCapsuleHalfHeight - CrouchCapsuleHalfHeight;

	//These 2 meshes are main part of modular mesh and they are constant for this project

	//Initialize and assign Head mesh
	USkeletalMeshComponent* Head = GetMesh();
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> HeadAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Agent/Mesh/Body/SKM_Head.SKM_Head'"));
	if (HeadAsset.Succeeded())
		Head->SetSkeletalMesh(HeadAsset.Object);
	Head->SetRelativeLocationAndRotation(FVector(0,0,-80),FRotator(0,-90,0));

	//Initialize and assign Arms mesh
	USkeletalMeshComponent* Arms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Arms"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ArmsAsset(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Agent/Mesh/Modules/SKM_Arms.SKM_Arms'"));
	if (ArmsAsset.Succeeded())
		Arms->SetSkeletalMesh(ArmsAsset.Object);

	Arms->SetLeaderPoseComponent(Head);
	Arms->SetupAttachment(Head);
	MeshModules.Add("Arms",Arms);

	//The level 1 & 2 is not combat intensive thus other meshes are variable and left to blueprint assignment
	USkeletalMeshComponent* Torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	Torso->SetLeaderPoseComponent(Head);
	Torso->SetupAttachment(Head);
	MeshModules.Add("Torso",Torso);
	
	USkeletalMeshComponent* Pants = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pants"));
	Pants->SetLeaderPoseComponent(Head);
	Pants->SetupAttachment(Head);
	MeshModules.Add("Pants",Pants);
	
	USkeletalMeshComponent* Vest = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Vest"));
	Vest->SetLeaderPoseComponent(Head);
	Vest->SetupAttachment(Head);
	MeshModules.Add("Vest",Vest);
	
	USkeletalMeshComponent* Belt = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Belt"));
	Belt->SetLeaderPoseComponent(Head);
	Belt->SetupAttachment(Head);
	MeshModules.Add("Belt",Belt);

	USkeletalMeshComponent* Patch = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Patch"));
	Patch->SetLeaderPoseComponent(Head);
	Patch->SetupAttachment(Head);
	MeshModules.Add("Patch", Patch);

}

void AAgentCharacter::BeginPlay() {
	Super::BeginPlay();
	UpdateHealth();
	StartingAim = GetBaseAimRotation();
	bIsMovementAllowed = true;
	ShowHUD(false,20.0f);
}


void AAgentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateAimOffset(DeltaTime);

	float NewSpeed = FMath::FInterpTo(
    GetCharacterMovement()->MaxWalkSpeed,
    TargetSpeed,
    DeltaTime,
    InterpSpeed
);
GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

if(Camera) 
{
	const float TargetFOV = bIsZooming ? ZoomedFOV : DefaultFOV;
	const float CurrentFOV = Camera->FieldOfView;
	
	const float NewFOV = FMath::FInterpTo(CurrentFOV,
		TargetFOV,
		DeltaTime,
		ZoomInterpSpeed);
		
		Camera->SetFieldOfView(NewFOV);
}
}

void AAgentCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Stealth)
		Stealth->SetCharacter(this);
}

//Bind keyboard to Input Handler component
void AAgentCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputHandler->BindInputActions(PlayerInputComponent);
}

void AAgentCharacter::CalculateAimOffset(float DeltaTime)
{
		FRotator ControlRotation = Controller->GetControlRotation();
		FRotator ActorRotation = GetActorRotation();

		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
		float ChangeSpeed = 3.0f;
		
		float TargetPitch = FMath::FInterpTo(AO_Pitch, DeltaRotation.Pitch, DeltaTime, ChangeSpeed);
		float TargetYaw = FMath::FInterpTo(AO_Yaw, DeltaRotation.Yaw, DeltaTime, ChangeSpeed);

		AO_Pitch = FMath::Clamp(TargetPitch, -90.0f, 90.0f);
		AO_Yaw = FMath::Clamp(TargetYaw, -90.0f, 90.0f);
}



//Makes character able to change its position by keyboard presses
void AAgentCharacter::MoveControl(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr && bIsMovementAllowed)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (!Stealth->GetIsCovering()) {
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
		else {
			AddMovementInput(RightDirection, MovementVector.X);
			AddMovementInput(RightDirection, MovementVector.Y);
		}

	}													
}

//Records mouse movements to look around
void AAgentCharacter::LookControl(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAgentCharacter::CrouchAction(const FInputActionValue& Value)
{
	if (!bIsCrouched)
		Stealth->Crouch();
	else
		Stealth->UnCrouch();
}

void AAgentCharacter::CoverAction(const FInputActionValue& Value)
{
	if (Stealth) {
		if(!Stealth->GetIsCovering())
			Stealth->Attach();
		else
			Stealth->Detach();
	}
}

void AAgentCharacter::SprintAction(const FInputActionValue& Value)
{
	if (!this->bIsCrouched && !Stealth->GetIsCovering() && !Combat->GetIsAiming()) {
		bIsSprinting = Value.Get<bool>();
		ChangeSpeed(bIsSprinting ? BaseSprintSpeed : BaseWalkSpeed);
	}
}

void AAgentCharacter::EquipAction(const FInputActionValue& Value)
{
	if (OverlappingWeapon)
		Combat->EquipWeapon(OverlappingWeapon);
}

void AAgentCharacter::FireAction(const FInputActionValue& Value)
{
}

void AAgentCharacter::AimAction(const FInputActionValue& Value)
{
	const bool bPresed = Value.Get<bool>();
	bIsAiming = bPresed;
	bIsZooming = bPresed;

}

void AAgentCharacter::Jump()
{
	//We shouldn't able to jump is character is aiming
	if (!(Combat == nullptr || (Combat && Combat->GetIsAiming())))
		Super::Jump();
}

//This function is depended to levels so it must be called by level blueprints
void AAgentCharacter::EquipArmor(bool bIsEquip)
{
	int32 index = 0;
	for (auto& elem : MeshModules)
	{
		if (index >= ArmorsThreshold) {
			elem.Value->SetVisibility(bIsEquip);
		}

		index++;
	}

	Health += 40;
}


void AAgentCharacter::ShowHUD(bool bShow, float ShowSpeed)
{
	if(!bShow){
		GetHUD()->HideHealthBar(ShowSpeed);
		GetHUD()->HideAmmoBar(ShowSpeed);
	} else {
		GetHUD()->ShowHealthBar(ShowSpeed);
		GetHUD()->ShowAmmoBar(ShowSpeed);
	}

}

void AAgentCharacter::SetOverlappingWeapon(AWeapon* weapon)
{
	OverlappingWeapon = weapon;
}

void AAgentCharacter::PlayReloadMontage(EWeaponType type)
{

}

void AAgentCharacter::PlayFireMontage()
{
}

void AAgentCharacter::PlayHitMontage()
{
}

void AAgentCharacter::UpdateHealth()
{
	GetHUD()->UpdateHealth(Health);
}
