// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_RocketLauncher.h"

#include "DrawDebugHelpers.h"
#include "RocketProjectile.h"
#include "Camera/CameraComponent.h"


void AWeapon_RocketLauncher::PreFireGun()
{
	
	Fire();
	
}

void AWeapon_RocketLauncher::OnFire()
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

void AWeapon_RocketLauncher::Fire()
{
	if (!MyPlayer->HasAuthority())
	{
		return;
	}
	
		aProjectileToSpawn = aProjectileRocket;

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
				MyProjectile = World->SpawnActor<ARocketProjectile>(aProjectileToSpawn, SpawnLocation, SpawnRotation, ActorSpawnParams);

				MyProjectile->SetOwner(MyPlayer);
				
				if (MyPlayer)
				{
					Cast<ARocketProjectile>(MyProjectile)->SetMyPlayer(MyPlayer);
				}
				
			
			}
		}
	//}
	
}

