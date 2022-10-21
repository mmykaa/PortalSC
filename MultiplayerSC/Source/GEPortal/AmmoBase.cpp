// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoBase.h"
#include "GEPortalCharacter.h"
#include "Weapon_AssaultRifle.h"
#include "Weapon_RocketLauncher.h"
#include "Weapon_Shotgun.h"

// Sets default values
AAmmoBase::AAmmoBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	DefaultRoot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(DefaultRoot);
    	
	SphereCollider=CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
    
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AAmmoBase::OnSphereBeginOverlap);
	SphereCollider->SetupAttachment(DefaultRoot);

	bFoundActor = false;
}

// Called when the game starts or when spawned
void AAmmoBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmoBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AAmmoBase::AmmoBase_GetAmmoType()
{
	TArray<AActor*> PlayerInventory = Cast<AGEPortalCharacter>(MyPlayer)->GetWeapons();

	switch (AmmoType)
	{
	case AmmoAssaultRifle:

		for (int i = 0; i < PlayerInventory.Num(); ++i)
		{
			if (Cast<AWeapon_AssaultRifle>(PlayerInventory[i]))
			{
				AmmoBase_DeliverAmmo(i);
			}
		}

		break;

	case AmmoShotgun:

		for (int i = 0; i < PlayerInventory.Num(); ++i)
		{
			if (Cast<AWeapon_Shotgun>(PlayerInventory[i]))
			{
				AmmoBase_DeliverAmmo(i);
			}
		}

		break;

	case AmmoRocketLauncher:

		for (int i = 0; i < PlayerInventory.Num(); ++i)
		{
			if (Cast<AWeapon_RocketLauncher>(PlayerInventory[i]))
			{
				AmmoBase_DeliverAmmo(i);
			}
		}

		break;
	}
}


void AAmmoBase::AmmoBase_DeliverAmmo(int WeaponIndex)
{
	Cast<AGEPortalCharacter>(MyPlayer)->ChooseWeaponToReceiveAmmo(WeaponIndex, iAmmoAmount);
	this->Destroy();
}

void AAmmoBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	if (Cast<AGEPortalCharacter>(OtherActor) && !bFoundActor)
	{
		MyPlayer = Cast<AGEPortalCharacter>(OtherActor);
		bFoundActor = true;
		AmmoBase_GetAmmoType();
	}
}

