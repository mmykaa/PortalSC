// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"

#include "AmmoBase.h"
#include "DrawDebugHelpers.h"
#include "GEPortalCharacter.h"
#include "Portal.h"
#include "WeaponBase.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ARocketProjectile::ARocketProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComponent);
	
	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 2500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	InitialLifeSpan = 10.0f;


	SphereCollider = CreateDefaultSubobject<USphereComponent>(FName("SphereCollider"));
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &ARocketProjectile::OnSphereBeginOverlap);
	SphereCollider->SetupAttachment(ProjectileMesh);

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(ProjectileMesh);
	fBlastRadius = 6000.0f;

	iOnExplosionDamageToApplyOnLimb = 11;
	iPlayersToDamage = 0;
}

void ARocketProjectile::BeginPlay()
{
	Super::BeginPlay();
}


void ARocketProjectile::SetMyPlayer(AGEPortalCharacter* Player)
{
	MyPlayer = Player;
}

void ARocketProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
									 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if ((!bActorFound && OtherActor != this && OtherActor != MyPlayer && !Cast<AAmmoBase>(OtherActor)) && !bExploded )
	{
		UE_LOG(LogTemp, Warning, TEXT("ACTOR FOUND %s"),*OtherActor->GetName());
		
		if(Cast<APortal>(OtherActor))
		{
			bActorFound = true;
			UE_LOG(LogTemp, Warning, TEXT("PORTAL Hit %s"),*OtherActor->GetName());
		
			GetWorld()->GetTimerManager().SetTimer(fthResetActor, this, &ARocketProjectile::ResetActor, 0.05f, true, 0.05f);
			Cast<APortal>(OtherActor)->RedirectProjectile(this, this->GetActorLocation(), GetActorRotation(), MyPlayer);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(fthResetActor);
			bActorFound = true;
			UE_LOG(LogTemp, Warning, TEXT("OTHER Hit %s"),*OtherActor->GetName());
			if (!bExploded)
			{
				SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SphereCollider->SetNotifyRigidBodyCollision(false);
				SphereCollider->SetGenerateOverlapEvents(false);
				SphereCollider->SetActive(false);

				ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				ProjectileMesh->SetNotifyRigidBodyCollision(false);
				ProjectileMesh->SetGenerateOverlapEvents(false);
				ProjectileMesh->SetActive(false);
				if (!IsPendingKill())
				{
					StartExplosion();
				}
			}
		}
	}
}



void ARocketProjectile::StartExplosion()
{
	//Start Explosion radius
	bExploded = true;
	CheckPlayersInZone();
}


void ARocketProjectile::CheckPlayersInZone()
{
	DamageSphere->SetSphereRadius(fBlastRadius);
	//if players were found add them to the array
	DamageSphere->GetOverlappingActors(ActorsFound, TSubclassOf<AGEPortalCharacter>());

	int iPlayersFound = 0;
	
	for (int i = 0; i < ActorsFound.Num(); ++i)
	{
		if (Cast<AGEPortalCharacter>(ActorsFound[i]))
		{
			++iPlayersFound;
		}
	}

	if (iPlayersFound == 0)
	{
		this->Destroy();
	}
	
	for (AActor* Player : ActorsFound)
	{
		if (AGEPortalCharacter* Character = Cast<AGEPortalCharacter>(Player))
		{
			++iPlayersToDamage;
			CheckPlayerDistance(Character);
		}
	}

	if (iPlayersToDamage == 0)
	{
		this->Destroy();
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("ACTORS FOUND: %d"), PlayersFound.Num());
}

void ARocketProjectile::CheckPlayerDistance(AGEPortalCharacter* PlayerToCheckDistance)
{
	//check the distance that the players are from the center
	float fDistance = UKismetMathLibrary::Vector_Distance(this->GetActorLocation(),PlayerToCheckDistance->GetActorLocation());
	float fMaxDistance = fBlastRadius/10.0f;
	float fDamageMultiplier = 1.0f;
	
	// 20% of the distance will always deal full damage
	if (fDistance <= (fMaxDistance* 0.20f))
	{
		//Apply the Default DamageMultiplier

		//UE_LOG(LogTemp, Warning, TEXT("FULL DAMGE"));
		//UE_LOG(LogTemp, Warning, TEXT("FULL DMULT: %f"), fDamageMultiplier);
		DamageCalculation(PlayerToCheckDistance, fDamageMultiplier);
	}
	else
	{
		//Apply the Adjusted DamageMultiplier
		//get the percentage from the center distance to the max distance
		//since we give full damage if the character has a distance of 20%
		//then, the damage must decrease from this value and not the center value
		
		fDamageMultiplier = (fDamageMultiplier*fMaxDistance*0.20f)/fDistance;
		//UE_LOG(LogTemp, Warning, TEXT("SOME DAMGE"));
		//UE_LOG(LogTemp, Warning, TEXT("SOME DMULT: %f"), fDamageMultiplier);
		DamageCalculation(PlayerToCheckDistance, fDamageMultiplier);
	}
}

void ARocketProjectile::DamageCalculation(AGEPortalCharacter* PlayerToDamage, float fDamagePercentage)
{
	//Check if the player is behind a wall or not
	//Calculate the damage based on that
	//Trace for each body part and check if they re exposed to the explosion
	float fDamageToDeal = 0.0f;
	--iPlayersToDamage;

	TArray<FVector> BoneLocations;
	BoneLocations.AddUninitialized(9);
	BoneLocations[0] = PlayerToDamage->GetMesh3P()->GetBoneLocation("head");
	BoneLocations[1] = PlayerToDamage->GetMesh3P()->GetBoneLocation("spine_02");
	BoneLocations[2] = PlayerToDamage->GetMesh3P()->GetBoneLocation("spine_01");
	BoneLocations[3] = PlayerToDamage->GetMesh3P()->GetBoneLocation("lowerarm_r");
	BoneLocations[4] = PlayerToDamage->GetMesh3P()->GetBoneLocation("lowerarm_l");
	BoneLocations[5] = PlayerToDamage->GetMesh3P()->GetBoneLocation("thigh_r");
	BoneLocations[6] = PlayerToDamage->GetMesh3P()->GetBoneLocation("thigh_l");
	BoneLocations[7] = PlayerToDamage->GetMesh3P()->GetBoneLocation("calf_r");
	BoneLocations[8] = PlayerToDamage->GetMesh3P()->GetBoneLocation("calf_l");
			
	for (int i = 0; i < BoneLocations.Num(); ++i)
	{
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		
		FVector vStart = GetActorLocation() - (this->GetActorForwardVector()* 50.0f);
		FVector vEnd = BoneLocations[i];
		
		GetWorld()->LineTraceSingleByChannel(HitResult, vStart, vEnd,ECC_Visibility, CollisionParams);
		DrawDebugLine(GetWorld(),vStart,vEnd, FColor::Red,false, 2.0f);
		
		if (Cast<AGEPortalCharacter>(HitResult.GetActor()))
		{
			bActorFound = true;
			fDamageToDeal = iOnExplosionDamageToApplyOnLimb * fDamagePercentage;
			int iRoundedDamage = UKismetMathLibrary::Round(fDamageToDeal);

			UE_LOG(LogTemp, Warning, TEXT("DamgeToApply %d"), iRoundedDamage);
			ApplyDamageToPlayer(PlayerToDamage, iRoundedDamage);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Missed"));
		}
	}

	this->Destroy();

}

void ARocketProjectile::ApplyDamageToPlayer(AGEPortalCharacter* PlayerToDamage, int iDamageToDeal)
{
		PlayerToDamage->DamagePlayer(iDamageToDeal, GetOwner());
		--iPlayersToDamage;
}


