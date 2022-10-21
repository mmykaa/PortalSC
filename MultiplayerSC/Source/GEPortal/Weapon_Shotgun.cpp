// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon_Shotgun.h"

#include "DrawDebugHelpers.h"
#include "Portal.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"


void AWeapon_Shotgun::PreFireGun()
{	
	OnFire();
}

void AWeapon_Shotgun::OnFire()
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

	Fire();
}

void AWeapon_Shotgun::Fire()
{
	
	for (int i = 0; i < iMaxSpreadShots; ++i)
	{
		FHitResult OutHit;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Cast<AActor>(MyPlayer));
		
		float fXEnd = UKismetMathLibrary::RandomFloatInRange(-fSpread, fSpread);
		float fYEnd = UKismetMathLibrary::RandomFloatInRange(-fSpread, fSpread);
		float fZEnd = UKismetMathLibrary::RandomFloatInRange(-fSpread, fSpread);
		
		FVector vSpreadedEnd = FVector(vFireEnd.X+fXEnd, vFireEnd.Y+fYEnd, vFireEnd.Z+fZEnd);

		GetWorld()->LineTraceSingleByChannel(OutHit, vFireStart, vSpreadedEnd, ECC_Visibility, CollisionParams);
		DrawDebugLine(GetWorld(), vFireStart, vSpreadedEnd, FColor::Emerald, false, 0.3f);

		if (Cast<AGEPortalCharacter>(OutHit.GetActor()))
		{
			AGEPortalCharacter* PlayerToDamage = Cast<AGEPortalCharacter>(OutHit.GetActor());
			
			PlayerToDamage->DamagePlayer(iDamageOnHit, MyPlayer);
		}

		if (Cast<APortal>(OutHit.GetActor()))
		{
			Cast<APortal>(OutHit.GetActor())->RedirectShot(OutHit, GetActorRotation(), GetMyPlayer());
		}

	}	
}

