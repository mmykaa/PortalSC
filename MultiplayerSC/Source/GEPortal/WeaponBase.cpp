// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "DrawDebugHelpers.h"
#include "GEPortalCharacter.h"
#include "GEPortalGameMode.h"
#include "WeaponsManager.h"
#include "Weapon_AssaultRifle.h"
#include "Weapon_PortalGun.h"
#include "Weapon_RocketLauncher.h"
#include "Weapon_Shotgun.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	DefaultRoot->SetMobility(EComponentMobility::Movable);
    SetRootComponent(DefaultRoot);
    	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMFPWeapon"));
	WeaponMesh->SetupAttachment(DefaultRoot);
	
	SphereCollider=CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
    
    SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnSphereBeginOverlap);
    SphereCollider->SetupAttachment(WeaponMesh);

	bFoundActor = false;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	AActor* WeaponManager = UGameplayStatics::GetActorOfClass(GetWorld(),AWeaponsManager::StaticClass());
	
	if (Cast<AWeapon_Shotgun>(this) || Cast<AWeapon_RocketLauncher>(this))
	{
		Cast<AWeaponsManager>(WeaponManager)->AddWeapon();
	}
}


// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AWeaponBase::WeaponBase_SetPlayer(AGEPortalCharacter* Player)
{
	MyPlayer = Player;
}

//Only Updates the ammo to the UI On ClientSide
//Without this the ammo is always the default value
void AWeaponBase::UpdateAmmoOnClient()
{
	if (iCurrentAmmo < 0)
	{
		iCurrentAmmo = 0;
	}

	if (iCurrentAmmo > 0)
	{
		--iCurrentAmmo;
	}

	if (iAmmoLeftInPouch == 0)
	{
		iClipSize = iCurrentAmmo;
	}

}

//Only Updates the ammo to the UI On ClientSide
//Without this the ammo is always the default value
void AWeaponBase::UpdateAmmoReloadOnClient()
{
	if (iAmmoLeftInPouch > 0)
	{
		iClipSize = iDefaultClipSize;

		if (iCurrentAmmo < iClipSize)
		{
			if (iAmmoLeftInPouch >= iClipSize)
			{
				int iAmountToReload = iClipSize - iCurrentAmmo;
				iAmmoLeftInPouch -= iAmountToReload;
				iCurrentAmmo += iAmountToReload;
			}
			else if	(iAmmoLeftInPouch < iClipSize)
			{
				if	(iAmmoLeftInPouch >= iClipSize - iCurrentAmmo)
				{
					int iAmountToReload = iClipSize - iCurrentAmmo;
				
					iCurrentAmmo += iAmountToReload;
					iAmmoLeftInPouch -= iAmountToReload;
					iClipSize = iCurrentAmmo;
				}
				else
				{
					int iAmountToReload = iAmmoLeftInPouch;
				
					iCurrentAmmo += iAmountToReload;
					iAmmoLeftInPouch -= iAmountToReload;
					iClipSize = iCurrentAmmo;
				}				
			}
		}

		if (iAmmoLeftInPouch < 0)
		{
			iAmmoLeftInPouch = 0;
		}
			
	}
}

void AWeaponBase::WeaponBase_Shoot(FVector Start, FVector Forward, FVector End, AActor* Player)
{
		
	vFireStart = Start;
	vFireForwardVector = Forward;
	vFireEnd = End;
	WeaponBase_SetPlayer(Cast<AGEPortalCharacter>(Player));
	
	//Check Which type of gun should be fired
	switch (ShootingType)
	{
		
	//Filter By Shooting Types
	case LineTrace:

		switch (WeaponType)
		{
		case AssaultRifle:
			WeaponBase_FireAR();

			break;

		case Shotgun:
			WeaponBase_FireShotgun();

			break;
		}

	//Filter By Shooting Types
	case Projectile:

		switch (WeaponType)
		{
		case RocketLauncher:
			WeaponBase_FireRocketLauncher();

			break;

		case PortalGun:
			WeaponBase_FirePortalGun();


			break;
		}
	}
}

void AWeaponBase::WeaponBase_FireAR()
{
	Cast<AWeapon_AssaultRifle>(WeaponToFire)->PreFireGun();
}

void AWeaponBase::WeaponBase_FireShotgun()
{
	Cast<AWeapon_Shotgun>(WeaponToFire)->PreFireGun();
}

void AWeaponBase::WeaponBase_FireRocketLauncher()
{
	Cast<AWeapon_RocketLauncher>(WeaponToFire)->PreFireGun();
}

void AWeaponBase::WeaponBase_FirePortalGun()
{
	Cast<AWeapon_PortalGun>(WeaponToFire)->OnFire();
}

bool AWeaponBase::WeaponBase_CheckAmmo(AActor* WeaponToCheck)
{
	WeaponToFire = WeaponToCheck;
	
	WeaponBase_CheckAmmoInWeapon();

	WeaponBase_StartFireRate();
	

	if (bCanFire)
	{
		--iCurrentAmmo;

		if (iAmmoLeftInPouch == 0)
		{
			iClipSize = iCurrentAmmo;
		}
	}
	Server_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);
	
	return bCanFire;
}

void AWeaponBase::WeaponBase_CheckAmmoInWeapon()
{
	switch (ReloadTypes)
	{
	case NeedsReload:

		switch (WeaponType)
		{
		case AssaultRifle:
			
			// Player is Using AssaultRifle
			WeaponBase_CheckCurrentWeaponAmmo();
			
			break;

		case Shotgun:
			// Player is Using Shotgun
			WeaponBase_CheckCurrentWeaponAmmo();
			break;

		case RocketLauncher:
			// Player is Using Rocket
			WeaponBase_CheckCurrentWeaponAmmo();
			break;
		}
		break;

	case NoReload:

		bCanFire = true;
		
		Server_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);
		
		break;
	}
}

bool AWeaponBase::WeaponBase_CheckFireRate()
{
	return bIsInCooldown;
}

void AWeaponBase::WeaponBase_StartFireRate()
{
	if (!bIsInCooldown)
	{
		GetWorld()->GetTimerManager().SetTimer(fthFireRateHandle, this,&AWeaponBase::WeaponBase_EndFireRate, fFireRate, false, fFireRate);
		UE_LOG(LogTemp, Warning, TEXT("IS IN COOLDOWN"));
		UE_LOG(LogTemp, Warning, TEXT("FireRate: %f"), fFireRate);
		bIsInCooldown = true;
		bCanFire = false;
	}
}

void AWeaponBase::WeaponBase_EndFireRate()
{
	UE_LOG(LogTemp, Warning, TEXT("IS NOT NOT NOT IN COOLDOWN"));
	GetWorld()->GetTimerManager().ClearTimer(fthFireRateHandle);
	bIsInCooldown = false;
	bCanFire = true;

}

void AWeaponBase::WeaponBase_CheckCurrentWeaponAmmo()
{
		if (iCurrentAmmo > 0 && !bIsReloading)
		{
			bCanFire = true;
		}
	
		if (iCurrentAmmo == 0 && !bIsReloading)
		{
			bCanFire = false;
		}

	Server_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);

}

void AWeaponBase::WeaponBase_AddAmmo(int Amount)
{
	iAmmoLeftInPouch += Amount;

	if (!MyPlayer->HasAuthority())
	{
		Server_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);
	}

}


//Imagine losing bullets that are in the clip when reloading...
//Reloads to the clip size
//keeps the current bullets in the clip, only reloading whats needed
//if there are no ammo left to reload, the clip size updates with the current ammo left in the clip
void AWeaponBase::WeaponBase_Reload()
{
		if (iAmmoLeftInPouch > 0)
		{
			iClipSize = iDefaultClipSize;

			if (iCurrentAmmo < iClipSize)
			{
				if (iAmmoLeftInPouch >= iClipSize)
				{
					int iAmountToReload = iClipSize - iCurrentAmmo;
					iAmmoLeftInPouch -= iAmountToReload;
					iCurrentAmmo += iAmountToReload;
				}
				else if	(iAmmoLeftInPouch < iClipSize)
				{
					if	(iAmmoLeftInPouch >= iClipSize - iCurrentAmmo)
					{
						int iAmountToReload = iClipSize - iCurrentAmmo;
				
						iCurrentAmmo += iAmountToReload;
						iAmmoLeftInPouch -= iAmountToReload;
						iClipSize = iCurrentAmmo;
					}
					else
					{
						int iAmountToReload = iAmmoLeftInPouch;
				
						iCurrentAmmo += iAmountToReload;
						iAmmoLeftInPouch -= iAmountToReload;
						iClipSize = iCurrentAmmo;
					}				
				}
			}

			if (iAmmoLeftInPouch < 0)
			{
				iAmmoLeftInPouch = 0;
			}
			
		}
	
	Server_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);
}

void AWeaponBase::Server_OnRep_Ammo_Implementation(int CurrentAmmo, int ClipSizeAmmo, int LeftAmmo,
	bool bCanFireState)
{
	iCurrentAmmo = CurrentAmmo;
	iClipSize = ClipSizeAmmo;
	iAmmoLeftInPouch = LeftAmmo;
	bCanFire = bCanFireState;

	Client_OnRep_Ammo(iCurrentAmmo, iClipSize, iAmmoLeftInPouch, bCanFire);
}


void AWeaponBase::Client_OnRep_Ammo_Implementation(int CurrentAmmo, int ClipSizeAmmo, int LeftAmmo, bool bCanFireState)
{
	iCurrentAmmo = CurrentAmmo;
	iClipSize = ClipSizeAmmo;
	iAmmoLeftInPouch = LeftAmmo;
	bCanFire = bCanFireState;
}

void AWeaponBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
									   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	//Prevent the player to change weapon while is reloading if he steps on it we dont give it
	
	//this will prevent the player to reload a weapon that he is not holding, as so he cant reload a weapon and
	//switch to another, reseting the ammo on the previous weapon and being ready to fire the weapon that he just picked up
	
	if (Cast<AGEPortalCharacter>(OtherActor) && !bFoundActor && !bIsReloading)
	{
		MyPlayer = Cast<AGEPortalCharacter>(OtherActor);

		bFoundActor = true;
		if (MyPlayer->HasAuthority())
		{
			MyPlayer->CheckIfWeaponIsAlreadyCollected(this);
		
			if (MyPlayer->CanPickupWeapon())
			{
				MyPlayer->AddWeaponToPlayerInventory(this);
				SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				SphereCollider->SetNotifyRigidBodyCollision(false);
				SphereCollider->SetGenerateOverlapEvents(false);
				SphereCollider->SetActive(false);
				WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				
				AActor* WeaponManager = UGameplayStatics::GetActorOfClass(GetWorld(),AWeaponsManager::StaticClass());
			
				if (Cast<AWeapon_Shotgun>(this) || Cast<AWeapon_RocketLauncher>(this))
				{
					Cast<AWeaponsManager>(WeaponManager)->RemoveWeapon();
				}
			}
		}
		else
		{
			Server_OnRep_WeaponPickup();
		}
	}
}


void AWeaponBase::Server_OnRep_WeaponPickup_Implementation()
{
	MyPlayer->CheckIfWeaponIsAlreadyCollected(this);
		
	if (MyPlayer->CanPickupWeapon())
	{
		MyPlayer->AddWeaponToPlayerInventory(this);

		SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereCollider->SetNotifyRigidBodyCollision(false);
		SphereCollider->SetGenerateOverlapEvents(false);
		SphereCollider->SetActive(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AActor* WeaponManager = UGameplayStatics::GetActorOfClass(GetWorld(),AWeaponsManager::StaticClass());
			
		if (Cast<AWeapon_Shotgun>(this) || Cast<AWeapon_RocketLauncher>(this))
		{
			Cast<AWeaponsManager>(WeaponManager)->RemoveWeapon();
		}
	}
	Client_OnRep_WeaponPickup();
}


void AWeaponBase::Client_OnRep_WeaponPickup_Implementation()
{
	MyPlayer->CheckIfWeaponIsAlreadyCollected(this);
		
	if (MyPlayer->CanPickupWeapon())
	{
		MyPlayer->AddWeaponToPlayerInventory(this);

		SphereCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereCollider->SetNotifyRigidBodyCollision(false);
		SphereCollider->SetGenerateOverlapEvents(false);
		SphereCollider->SetActive(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
		AActor* WeaponManager = UGameplayStatics::GetActorOfClass(GetWorld(),AWeaponsManager::StaticClass());
			
		if (Cast<AWeapon_Shotgun>(this) || Cast<AWeapon_RocketLauncher>(this))
		{
			Cast<AWeaponsManager>(WeaponManager)->RemoveWeapon();
		}
	}
}

void AWeaponBase::Server_OnRep_FireRateCooldown_Implementation(bool State)
{
	Client_OnRep_FireRateCooldown(State);
}

void AWeaponBase::Client_OnRep_FireRateCooldown_Implementation(bool State)
{
	bIsInCooldown = State;
}

