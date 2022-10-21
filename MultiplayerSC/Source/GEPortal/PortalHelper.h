// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalHelper.generated.h"

UCLASS()
class GEPORTAL_API APortalHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortalHelper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY() 
	class AGEPortalCharacter* aPlayer;
	
	AGEPortalCharacter* GetExistingPlayer();
	
	void SetPlayer(AGEPortalCharacter* aExistingPlayer);

	UPROPERTY() TSubclassOf<class APortal> cPortalClass;

	//Spawns BluePortal
	void SpawnBluePortal(const FHitResult& Hit, AActor* Projectile);

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AActor> aBluePortalToSpawn;

	APortal* aBluePortalSpawned;

	//Spawns OrangePortal
	void SpawnOrangePortal(const FHitResult& Hit, AActor* Projectile);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AActor> aOrangePortalToSpawn;
	APortal* aOrangePortalSpawned;

	//Updates portal targets
	void UpdatePortalTargets();

	void CheckPortalPlacement(APortal* PortalToCheckLocation, FVector ProjectileImpactLocation);

	void FixPortalPlacementInWall(APortal* PortalToFixLocation, FVector Upper, FVector Right, FVector Bottom, FVector Left ,FVector ProjectileImpactLocation);

	FVector PortalBoxCenter;
	UPROPERTY() TArray<FVector> PortalBoxCorners;


	void LineTraceFromCenter(FVector Center, APortal* PortalToFixLocation);
	void LineTraceFromCorner(FVector Corner, APortal* PortalToFixLocation, FVector ProjectileImpactLocation);
	AActor* WallFound;

	void AdjustLocation(APortal* PortalToFixLocation, FVector CornerToAdjust, FVector ProjectileImpactLocation, FHitResult Hit);
	float maxAdjustbleSteps;
	float fAdjustmentSubstep;
	float fPortalHeight;
	
};
