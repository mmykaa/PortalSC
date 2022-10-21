// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponsManager.generated.h"

UCLASS()
class GEPORTAL_API AWeaponsManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponsManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	
#pragma region Ammo & Weapon Respawning

	UPROPERTY() float fTimeToRespawnAmmo;
	UPROPERTY() float fTimeToRespawnWeapon;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<TSubclassOf<AActor>> WeaponTypes;
	TArray<class AWeaponSpawnPoint*> WeaponPossibleSpawnPoints;	
	UPROPERTY() int iWeaponsInWorld;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> PortalGun;
	TArray<class APortalGunSpawnPoint*> PortalGunSpawnPoint;	

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<TSubclassOf<AActor>> AmmoTypes;
	TArray<class AAmmoSpawnPoint*> AmmoPossibleSpawnPoints;
	TArray<class AAmmoBase*> AmmosFound;
	
	

	////////////////////////////////////////////////////////////////////////
	void AddWeapon();
	void RemoveWeapon();

	void CheckIfAllWeaponsHaveBeenPickedUp();
	void CheckIfAllAmmoHaveBeenPickedUp();
	void SpawnPortalGun();
	
	void SpawnAmmoTypes();
	void SpawnWeaponTypes();
	

#pragma endregion Ammo & Weapon Respawning

	

	
};
