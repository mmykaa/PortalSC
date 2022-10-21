// Fill out your copyright notice in the Description page of Project Settings.


#include "GEBluePortalProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GEPortalGameMode.h"
#include "PortalHelper.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGEBluePortalProjectile::AGEBluePortalProjectile()
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

void AGEBluePortalProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// set up a notification for when this component hits something blocking
	CollisionComp->OnComponentHit.AddDynamic(this, &AGEBluePortalProjectile::OnHit); 
}

void AGEBluePortalProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                    FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->ActorHasTag(TEXT("PortableWall")) && OtherActor != this)
	{
		//Spawn BluePortal
		AActor* PortalHelper = UGameplayStatics::GetActorOfClass(GetWorld(), APortalHelper::StaticClass());
		Cast<APortalHelper>(PortalHelper)->SpawnBluePortal(Hit, this);
	}
	else if (!OtherActor->ActorHasTag(TEXT("PortableWall")))
	{
		Destroy();
	}
}
