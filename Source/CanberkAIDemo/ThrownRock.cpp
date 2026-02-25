// © 2025-2026 Canberk. All Rights Reserved.

#include "ThrownRock.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AThrownRock::AThrownRock()
{
	PrimaryActorTick.bCanEverTick = false;

	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	SetRootComponent(RockMesh);
	RockMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RockMesh->SetNotifyRigidBodyCollision(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 3000.f;
}

void AThrownRock::Launch(FVector Direction, float Speed)
{
	ProjectileMovement->Velocity = Direction * Speed;

	RockMesh->OnComponentHit.AddDynamic(this, &AThrownRock::OnRockHit);
}

void AThrownRock::OnRockHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasImpacted) return;
	bHasImpacted = true;

	FVector ImpactPoint = Hit.ImpactPoint;

	// Generate noise for AI hearing
	APawn* InstigatorPawn = Cast<APawn>(GetInstigator());
	if (InstigatorPawn)
	{
		MakeNoise(NoiseLoudness, InstigatorPawn, ImpactPoint);
	}

	// Play impact sound
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, ImpactPoint);
	}

	// Stop movement
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->ProjectileGravityScale = 0.f;

	// Destroy after delay
	SetLifeSpan(SelfDestructDelay);
}
