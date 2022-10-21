// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_AssaultRifle.h"

#include "DrawDebugHelpers.h"
#include "GEPortalCharacter.h"
#include "Portal.h"
#include "Camera/CameraComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


void AWeapon_AssaultRifle::PreFireGun()
{	
	OnFire();
}

void AWeapon_AssaultRifle::OnFire()
{
	//try and play a firing animation if specified
	if (MyPlayer)
	{
		if (FireAnimation != nullptr)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = MyPlayer->GetMesh1P()->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}	
	}
	
	Fire();
}

void AWeapon_AssaultRifle::Fire()
{
	
		FHitResult OutHit;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Cast<AActor>(MyPlayer));
		GetWorld()->LineTraceSingleByChannel(OutHit, vFireStart, vFireEnd, ECC_Visibility, CollisionParams);
		DrawDebugLine(GetWorld(), vFireStart, vFireEnd, FColor::Emerald, false, 0.2f);


		if (Cast<AGEPortalCharacter>(OutHit.GetActor()))
		{
		    AGEPortalCharacter* PlayerToDamage = Cast<AGEPortalCharacter>(OutHit.GetActor());
			
			
		    if (OutHit.BoneName.ToString() == FString("head"))
		    {
		    	PlayerToDamage->DamagePlayer(PlayerToDamage->GetMaxHealth(), MyPlayer);
		    }
		    else
		    {
		    	PlayerToDamage->DamagePlayer(iDamageOnHit, MyPlayer);
		    }
		}

		if (Cast<APortal>(OutHit.GetActor()))
		{
			Cast<APortal>(OutHit.GetActor())->RedirectShot(OutHit, GetActorRotation(), GetMyPlayer());
		}

	//	Reset Fire Rate Timer
	//GetWorld()->GetTimerManager().SetTimer(fthShotgunTimerHandle, this, &APlayerBehaviour::ResetShotgunFireRate, fShotgunFireRate, false);

}

