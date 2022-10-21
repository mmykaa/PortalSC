// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Weapon_PortalGun.generated.h"

/**
 * 
 */


UCLASS()
class GEPORTAL_API AWeapon_PortalGun : public AWeaponBase
{
	GENERATED_BODY()

public:
	
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay) UAnimMontage* FireAnimation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> aProjectileBlue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> aProjectileOrange;

	void FirePortalProjectileBlue();
	void FirePortalProjectileOrange();

	void PreFireGun();
	void OnFire();
	
};
