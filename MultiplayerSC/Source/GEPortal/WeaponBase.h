// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEPortalCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"


UENUM(BlueprintType)
enum EWeaponType
{
	AssaultRifle     UMETA(DisplayName = "AssaultRifle"),
	Shotgun			 UMETA(DisplayName = "Shotgun"),
	RocketLauncher   UMETA(DisplayName = "RocketLauncher"),
	PortalGun		 UMETA(DisplayName = "PortalGun"),
};

UENUM(BlueprintType)
enum EShootingType
{
	LineTrace      UMETA(DisplayName = "LineTrace"),
	Projectile     UMETA(DisplayName = "Projectile"),
	
};

UENUM(BlueprintType)
enum EReloadTypes
{
	NeedsReload   UMETA(DisplayName = "NeedsReload"),
	NoReload      UMETA(DisplayName = "NoReload"),
	
};



UCLASS()
class GEPORTAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY() AGEPortalCharacter* MyPlayer;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TEnumAsByte<EWeaponType> WeaponType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TEnumAsByte<EShootingType> ShootingType;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TEnumAsByte<EReloadTypes> ReloadTypes;
	
	UPROPERTY() AActor* WeaponToFire;
	UPROPERTY(BlueprintReadOnly,EditAnywhere) TSubclassOf<AActor> aProjectileToSpawn;

	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iDefaultCurrentAmmo;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iCurrentAmmo;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iDefaultClipSize;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iClipSize;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iAmmoLeftInPouch;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iDefaultAmmoLeftInPouch;

	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iDamageOnHit;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iMinRadialDamageOnHit;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere) float fReloadTime;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) float fFireRate;
	UPROPERTY() FTimerHandle fthFireRateHandle;
	UPROPERTY() bool bIsInCooldown;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USceneComponent*  DefaultRoot;
	UPROPERTY(BlueprintReadWrite,EditAnywhere) USkeletalMeshComponent* WeaponMesh;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USphereComponent * SphereCollider;
	UPROPERTY() bool bFoundActor;
	
	UPROPERTY() bool bIsReloading;
	UPROPERTY() bool bCanFire;
	UPROPERTY() FVector vFireStart;
	UPROPERTY() FVector vFireForwardVector;
	UPROPERTY() FVector vFireEnd;
	
	UFUNCTION() void WeaponBase_Shoot(FVector Start, FVector Forward, FVector End, AActor* Player);
	UFUNCTION() void WeaponBase_FireAR();
	UFUNCTION() void WeaponBase_FireShotgun();
	UFUNCTION() void WeaponBase_FireRocketLauncher();
	UFUNCTION() void WeaponBase_FirePortalGun();
	
	UFUNCTION() bool WeaponBase_CheckAmmo(AActor* WeaponToCheck);
	UFUNCTION() void WeaponBase_CheckAmmoInWeapon();
	UFUNCTION() void WeaponBase_CheckCurrentWeaponAmmo();
	
	UFUNCTION() bool WeaponBase_CheckFireRate();
	UFUNCTION() void WeaponBase_StartFireRate();
	UFUNCTION() void WeaponBase_EndFireRate();
	
	UFUNCTION() void WeaponBase_AddAmmo(int Amount);

	UFUNCTION() void WeaponBase_Reload();

	UFUNCTION() bool WeaponBase_GetIsReloading() const { return bIsReloading; }
	
	UFUNCTION() void WeaponBase_SetPlayer(AGEPortalCharacter* Player);
	UFUNCTION() void UpdateAmmoOnClient();
	UFUNCTION() void UpdateAmmoReloadOnClient();
	UFUNCTION(BlueprintCallable) int CurrentAmmoToUI() const { return iCurrentAmmo; }
	UFUNCTION(BlueprintCallable) int ClipAmmoToUI() const { return iClipSize; }
	UFUNCTION(BlueprintCallable) int LeftAmmoToUI() const { return iAmmoLeftInPouch; }

	UFUNCTION() USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	UFUNCTION() AGEPortalCharacter* GetMyPlayer() const { return MyPlayer; }
	UFUNCTION() void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
	UFUNCTION(Server, Reliable) void Server_OnRep_Ammo(int CurrentAmmo, int ClipSizeAmmo, int LeftAmmo, bool bCanFireState);
	UFUNCTION(Client, Reliable) void Client_OnRep_Ammo(int CurrentAmmo, int ClipSizeAmmo, int LeftAmmo, bool bCanFireState);

	
	UFUNCTION(Server, Reliable) void Server_OnRep_WeaponPickup();
	UFUNCTION(Client, Reliable) void Client_OnRep_WeaponPickup();

	UFUNCTION(Server, Reliable) void Server_OnRep_FireRateCooldown(bool State);
	UFUNCTION(Client, Reliable) void Client_OnRep_FireRateCooldown(bool State);
};
