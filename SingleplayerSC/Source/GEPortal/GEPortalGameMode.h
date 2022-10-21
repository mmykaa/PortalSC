// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "GEPortalGameMode.generated.h"

UCLASS(minimalapi)
class AGEPortalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGEPortalGameMode();

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

	void CheckIfAdjustWorked();


	UFUNCTION()
		 FVector ConvertLocation(FVector Location, AActor* Portal, AActor* Target);

	UFUNCTION()
		 FRotator ConvertRotation(FRotator Rotation, AActor* Portal, AActor* Target);

	UFUNCTION()
		 bool CheckIfIsInFrontOfPortal(FVector Point, FVector PortalLocation, FVector PortalNormal);

	UFUNCTION()
		 bool CheckIfThePlayerIsCrossingThePortal(FVector Point, FVector PortalLocation, FVector PortalNormal, bool out_LastInFront, FVector out_LastPosition);


	



};


