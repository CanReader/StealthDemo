// © 2025 Canberk. All Rights Reserved.

#include "PickupRock.h"
#include "AgentCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"

APickupRock::APickupRock()
{
	PrimaryActorTick.bCanEverTick = false;

	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	SetRootComponent(RockMesh);
	RockMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	RockMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	RockMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetSphereRadius(100.f);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void APickupRock::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupRock::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &APickupRock::OnSphereEndOverlap);
}

void APickupRock::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAgentCharacter* Character = Cast<AAgentCharacter>(OtherActor);
	if (Character)
	{
		Character->SetOverlappingPickupRock(this);
		if (PickupWidget)
		{
			PickupWidget->SetVisibility(true);
		}
	}
}

void APickupRock::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAgentCharacter* Character = Cast<AAgentCharacter>(OtherActor);
	if (Character)
	{
		Character->SetOverlappingPickupRock(nullptr);
		if (PickupWidget)
		{
			PickupWidget->SetVisibility(false);
		}
	}
}
