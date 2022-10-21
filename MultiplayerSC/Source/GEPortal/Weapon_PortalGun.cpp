// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_PortalGun.h"

#include "DrawDebugHelpers.h"
#include "GEBluePortalProjectile.h"
#include "GEOrangePortalProjectile.h"
#include "Camera/CameraComponent.h"



void AWeapon_PortalGun::PreFireGun()
{
	//Check For Firerate
}


void AWeapon_PortalGun::OnFire()
{
	// try and play a firing animation if specified
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



void AWeapon_PortalGun::FirePortalProjectileBlue()
{
	if (MyPlayer->HasAuthority())
	{
		aProjectileToSpawn = aProjectileBlue;
	
		if (aProjectileToSpawn != nullptr)
		{

			UWorld* const World = GetWorld();
			
			if (World != nullptr)
			{
				OnFire();

				const FRotator SpawnRotation = MyPlayer->GetPlayerControlRotation();

				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((MyPlayer->GetMuzzle() != nullptr) ? MyPlayer->GetMuzzle()->GetComponentLocation() : GetActorLocation()) +
																						SpawnRotation.RotateVector(MyPlayer->GetGunOffset());
				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<AGEBluePortalProjectile>(aProjectileToSpawn, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}
}


void AWeapon_PortalGun::FirePortalProjectileOrange()
{
	aProjectileToSpawn = aProjectileOrange;
	
		if (aProjectileToSpawn != nullptr)
		{
			UWorld* const World = GetWorld();
			if (World != nullptr)
			{
				OnFire();

				const FRotator SpawnRotation = MyPlayer->GetPlayerControlRotation();

				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((MyPlayer->GetMuzzle() != nullptr) ? MyPlayer->GetMuzzle()->GetComponentLocation() : GetActorLocation()) +
																						SpawnRotation.RotateVector(MyPlayer->GetGunOffset());

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<AGEOrangePortalProjectile>(aProjectileToSpawn, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}


