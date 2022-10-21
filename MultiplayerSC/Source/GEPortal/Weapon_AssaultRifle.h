// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEPortalCharacter.h"
#include "WeaponBase.h"
#include "Weapon_AssaultRifle.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API AWeapon_AssaultRifle : public AWeaponBase
{
	GENERATED_BODY()
	
public:

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay) UAnimMontage* FireAnimation;

	void PreFireGun();
	void OnFire();
	void Fire();

	int GetDamageOnHit() const {return iDamageOnHit;}

};
