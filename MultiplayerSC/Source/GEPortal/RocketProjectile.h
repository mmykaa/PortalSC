// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEPortalCharacter.h"
#include "GameFramework/Actor.h"
#include "RocketProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class GEPORTAL_API ARocketProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARocketProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly)
	USphereComponent* CollisionComp;
	bool bIsTravelling;
	FTimerHandle fthResetActor;
	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	
	UPROPERTY(VisibleDefaultsOnly) TArray<AActor*> ActorsFound;
	UPROPERTY(VisibleDefaultsOnly) AGEPortalCharacter* MyPlayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UStaticMeshComponent* ProjectileMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float fBlastRadius;

	UPROPERTY() int iOnExplosionDamageToApplyOnLimb;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) USphereComponent * SphereCollider;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USphereComponent * DamageSphere;
	bool bActorFound;
	bool bExploded;
	UPROPERTY() int iPlayersToDamage;
	
	///////////////////////////////////////////////////////////////////////////

	
	UFUNCTION() void StartExplosion();
	UFUNCTION() void CheckPlayersInZone();
	UFUNCTION() void CheckPlayerDistance(AGEPortalCharacter* PlayerToCheckDistance);
	UFUNCTION() void DamageCalculation(AGEPortalCharacter* PlayerToDamage, float fDamagePercentage);
	UFUNCTION() void ApplyDamageToPlayer(AGEPortalCharacter* PlayerToDamage, int iDamageToDeal);
	
	UFUNCTION() USphereComponent* GetCollisionComp() const { return CollisionComp; }
	
public:
	UFUNCTION() void SetMyPlayer(AGEPortalCharacter* Player);
	UFUNCTION() AGEPortalCharacter* GetMyPlayer() const { return MyPlayer; }
	UFUNCTION() void SetTravellingState(bool State)  { bIsTravelling = State; }
	UFUNCTION() void ResetActor() {bActorFound = false;}
	UFUNCTION()	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	UFUNCTION() void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
										 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
};






