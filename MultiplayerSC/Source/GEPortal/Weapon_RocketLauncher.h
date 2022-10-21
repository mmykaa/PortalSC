// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Weapon_RocketLauncher.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API AWeapon_RocketLauncher : public AWeaponBase
{
	GENERATED_BODY()

		
public:

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay) UAnimMontage* FireAnimation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> aProjectileRocket;
	AActor* MyProjectile;

	
	void PreFireGun();
	void OnFire();
	void Fire();
	void Reload();

};
