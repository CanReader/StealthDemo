// © 2024 Gamegine. All Rights Reserved.

#include "Weapon.h"
#include "AgentCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CharacterHUD.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(Mesh);
	Silencer  = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Addition"));
	Silencer->SetupAttachment(Mesh, FName("Suppressor"));

	Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);

	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Silencer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	PickupWidget->SetVisibility(false);

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetHUDAmmo()
{
	AAgentCharacter* Character = Cast<AAgentCharacter>(GetOwner());
	if (Character)
	{
		auto controller = GetWorld()->GetFirstPlayerController();
		if (AHUD* HUD = controller->GetHUD())
		{
			ACharacterHUD* CHUD = Cast<ACharacterHUD>(HUD);
		}
	}

}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
		PickupWidget->SetVisibility(bShowWidget);
}

void AWeapon::Fire(const FVector& HitTarget)
{
	auto Character = Cast<ABaseCharacter>(GetOwner());

	UGameplayStatics::PlaySoundAtLocation(GetWorld(),FireSound,GetActorLocation(),SoundIntensity);
	if(HasSilencer())
		MakeNoise(SoundIntensity/2,Character,GetActorLocation());
	else
		MakeNoise(SoundIntensity*2,Character,GetActorLocation());
	auto muzzle = Mesh->GetSocketByName(FName("Muzzle"));

	if (muzzle)
	{
		FTransform SocketTransform = muzzle->GetSocketTransform(Mesh);
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		if (AActor* actor = FireHit.GetActor()) {
			bool isHeadShot = FireHit.BoneName.IsEqual(FName("head"));
			UGameplayStatics::ApplyDamage(actor,isHeadShot ? HeadshotDamage : Damage,Character->Controller,this,UDamageType::StaticClass());
		}

	}
}

void AWeapon::Dropped()
{
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	return FVector();
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAgentCharacter* Character = Cast<AAgentCharacter>(OtherActor);
	if (Character)
	{
		Character->SetOverlappingWeapon(this);
		ShowPickupWidget(true);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAgentCharacter* Character = Cast<AAgentCharacter>(OtherActor);
	if (Character)
	{
		Character->SetOverlappingWeapon(nullptr);
		ShowPickupWidget(false);
	}
}


void AWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
	}
}