// Fill out your copyright notice in the Description page of Project Settings.


#include "GEOrangePortalProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GEPortalGameMode.h"

// Sets default values
AGEOrangePortalProjectile::AGEOrangePortalProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AGEOrangePortalProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentHit.AddDynamic(this, &AGEOrangePortalProjectile::OnHit);		// set up a notification for when this component hits something blocking
}

void AGEOrangePortalProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor->ActorHasTag(TEXT("PortableWall")) && OtherActor != this)
	{
		AGEPortalGameMode* PortalGameMode = GetWorld()->GetAuthGameMode<AGEPortalGameMode>();

		PortalGameMode->SpawnOrangePortal(Hit, this);
	}
	else if (!OtherActor->ActorHasTag(TEXT("PortableWall")))
	{
		Destroy();
	}
}