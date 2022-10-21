// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponsManager.h"

#include "AmmoBase.h"
#include "AmmoSpawnPoint.h"
#include "EngineUtils.h"
#include "PortalGunSpawnPoint.h"
#include "WeaponSpawnPoint.h"

// Sets default values
AWeaponsManager::AWeaponsManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	iWeaponsInWorld = 0;
}

// Called when the game starts or when spawned
void AWeaponsManager::BeginPlay()
{
	Super::BeginPlay();

	CheckIfAllAmmoHaveBeenPickedUp();
	
	CheckIfAllWeaponsHaveBeenPickedUp();
	
	SpawnPortalGun();
	
	FTimerHandle WeaponRespawn;
        		GetWorld()->GetTimerManager().SetTimer(WeaponRespawn, this, &AWeaponsManager::CheckIfAllWeaponsHaveBeenPickedUp,
        										   60.0f, true, 0.0f);
	
		FTimerHandle AmmoRespawn;
    		GetWorld()->GetTimerManager().SetTimer(AmmoRespawn, this, &AWeaponsManager::CheckIfAllAmmoHaveBeenPickedUp,
    										   60.0f, true, 0.0f);
	
}

void AWeaponsManager::AddWeapon()
{
	++iWeaponsInWorld;
}

void AWeaponsManager::RemoveWeapon()
{
	--iWeaponsInWorld;	
}

void AWeaponsManager::CheckIfAllWeaponsHaveBeenPickedUp()
{
	UE_LOG(LogTemp, Warning, TEXT("CHECKING WEAPON"));
	
	if (iWeaponsInWorld == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning WEAPON"));

		SpawnWeaponTypes();
	}
}

void AWeaponsManager::CheckIfAllAmmoHaveBeenPickedUp()
{
    UE_LOG(LogTemp, Warning, TEXT("CHECKING Ammo"));
	if (AmmosFound.Num() > 0)
	{
		AmmosFound.Empty();
	}
	
	UClass * AmmoClass = AAmmoBase::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(), AmmoClass); Actor; ++Actor )
	{
		AmmosFound.Add(Cast<AAmmoBase>(*Actor));
	}

	if (AmmosFound.Num() == 0)
	{
		SpawnAmmoTypes();
		UE_LOG(LogTemp, Warning, TEXT("Spawning Ammo"));

	}
}

void AWeaponsManager::SpawnPortalGun()
{
	UClass * PortalGunSpawnPointClass = APortalGunSpawnPoint::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(),PortalGunSpawnPointClass); Actor; ++Actor )
	{
		PortalGunSpawnPoint.Add(Cast<APortalGunSpawnPoint>(*Actor));
	}

	FActorSpawnParameters SpawnParams;

	GetWorld()->SpawnActor<AActor>(PortalGun,PortalGunSpawnPoint[0]->GetActorLocation(),PortalGunSpawnPoint[0]->GetActorRotation(),SpawnParams);
}

void AWeaponsManager::SpawnAmmoTypes()
{
	UClass * AmmoSpawnPointClass = AAmmoSpawnPoint::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(),AmmoSpawnPointClass); Actor; ++Actor )
	{
		AmmoPossibleSpawnPoints.Add(Cast<AAmmoSpawnPoint>(*Actor));
	}

	FActorSpawnParameters SpawnParams;
	
	for (int i = 0; i < AmmoTypes.Num(); ++i)
	{
		GetWorld()->SpawnActor<AActor>(AmmoTypes[i],AmmoPossibleSpawnPoints[i]->GetActorLocation(),AmmoPossibleSpawnPoints[i]->GetActorRotation(),SpawnParams);
	}
}

void AWeaponsManager::SpawnWeaponTypes()
{

	UClass * WeaponSpawnPointClass = AWeaponSpawnPoint::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(),WeaponSpawnPointClass); Actor; ++Actor )
	{
		WeaponPossibleSpawnPoints.Add(Cast<AWeaponSpawnPoint>(*Actor));
	}

	FActorSpawnParameters SpawnParams;

	for (int i = 0; i < WeaponTypes.Num(); ++i)
	{
		GetWorld()->SpawnActor<AActor>(WeaponTypes[i],WeaponPossibleSpawnPoints[i]->GetActorLocation(),WeaponPossibleSpawnPoints[i]->GetActorRotation(),SpawnParams);
	}
}



