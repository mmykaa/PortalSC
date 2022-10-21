// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalCharacter.h"

#include "CosmeticsHelper.h"
#include "GEPortalGameMode.h"
#include "GEPortalHUD.h"


#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Camera/PlayerCameraManager.h"

#include "DrawDebugHelpers.h"
#include "GEPlayerState.h"
#include "RespawnHelper.h"
#include "Weapon_AssaultRifle.h"
#include "Weapon_PortalGun.h"
#include "WeaponBase.h"
#include "WeaponsManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


AGEPortalCharacter::AGEPortalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(15.0f, 70.0f);
	
	// set our turn rates for input
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	
	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(RootComponent);

	// Create a gun mesh component
	FP_Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Weapon->SetOnlyOwnerSee(true);			
	FP_Weapon->bCastDynamicShadow = false;
	FP_Weapon->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Weapon->SetupAttachment(RootComponent);

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));

	TP_Gun->SetOwnerNoSee(true);			// otherwise won't be visible in the multiplayer
	TP_Gun->SetupAttachment(RootComponent);
	
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Weapon);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	bCanFirePortal = true;
	
	//Player Health
	iMaxHealth = 100;
	iCurrentHealth = iMaxHealth;
	bIsPlayerDead = false;
	fRespawnTimer = 1.0f;
}

void AGEPortalCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	TP_Gun->AttachToComponent(Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	FP_Weapon->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	
	Mesh1P->SetHiddenInGame(false, true);
	
	GetWorld()->GetTimerManager().SetTimer(fthCheckForPortableWalls, this, &AGEPortalCharacter::CheckIfCanFirePortals, 0.001f, true, 0.0f);
	SetDefaultWeapon();
	AddHUDToViewport();
	
	Client_OnRep_CurrentHealth(iCurrentHealth);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGEPortalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireOne", IE_Released, this, &AGEPortalCharacter::OnFire);
	PlayerInputComponent->BindAction("FireTwo", IE_Pressed, this, &AGEPortalCharacter::FirePortalGunOrange);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AGEPortalCharacter::ReloadWeapon);

	PlayerInputComponent->BindAction("SwitchUp", IE_Released, this, &AGEPortalCharacter::SwitchUpWeapon);
	PlayerInputComponent->BindAction("SwitchDown", IE_Released, this, &AGEPortalCharacter::SwitchDownWeapon);
	
	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGEPortalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGEPortalCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGEPortalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGEPortalCharacter::LookUpAtRate);
	
}


void AGEPortalCharacter::DisablePlayerInput()
{
	this->DisableInput(Cast<APlayerController>(this));
	this->GetCharacterMovement()->SetMovementMode(MOVE_None);
}

void AGEPortalCharacter::EnablePlayerInput()
{
	this->EnableInput(Cast<APlayerController>(this));
	this->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AGEPortalCharacter::UpdateCosmeticsOnLogIn()
{
	Server_OnRep_BodyColor(GetMesh3P()->GetMaterial(0));
}

FVector AGEPortalCharacter::GetGunOffset()
{
	return GunOffset;
}

USceneComponent* AGEPortalCharacter::GetMuzzle()
{
	return FP_MuzzleLocation;
}

FRotator AGEPortalCharacter::GetPlayerControlRotation()
{
	return GetControlRotation();
}

void AGEPortalCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGEPortalCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGEPortalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGEPortalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


#pragma region Weapon

void AGEPortalCharacter::OnFire()
{
	if (bIsPlayerDead)
	{
		return;
	}
	
	FVector vStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector vForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector vEnd = (vStart + (vForwardVector * 5000.0f));

	if(aCurrentWeapon == nullptr)
	{
		return;
	}
	
	if (HasAuthority())
	{
	    if(Cast<AWeapon_PortalGun>(aCurrentWeapon))
	    {
		    FirePortalGunBlue();
		}
	    else
	    {
	    	AWeaponBase * myWeapon = Cast<AWeaponBase>(aCurrentWeapon);
	        if (myWeapon->WeaponBase_CheckFireRate())
	        {
		        return;
	        }
	    	myWeapon->WeaponBase_StartFireRate();
	    	if (myWeapon->WeaponBase_CheckAmmo(aCurrentWeapon))
	    	{
	    		myWeapon->WeaponBase_Shoot(vStart, vForwardVector, vEnd, this);
	    	}
	    }
	}

	if (!HasAuthority())
	{
		Server_OnRep_OnFire(vStart, vForwardVector, vEnd, this);
	}
}

void AGEPortalCharacter::FirePortalGunBlue()
{
	if (bCanFirePortal)
	{
		if (Cast<AWeapon_PortalGun>(aCurrentWeapon))
		{
			Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
			Cast<AWeapon_PortalGun>(aCurrentWeapon)->FirePortalProjectileBlue();
		}
	}
}

void AGEPortalCharacter::FirePortalGunOrange()
{
	if (HasAuthority())
	{
		if (bCanFirePortal)
		{
			if (Cast<AWeapon_PortalGun>(aCurrentWeapon))
			{
				Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
				Cast<AWeapon_PortalGun>(aCurrentWeapon)->PreFireGun();
				Cast<AWeapon_PortalGun>(aCurrentWeapon)->FirePortalProjectileOrange();
			}
		}
	}
	if (!HasAuthority())
	{
		Server_OnRep_ShootOrangePortal();
	}
}

void AGEPortalCharacter::CheckIfCanFirePortals()
{
	if (Cast<AWeapon_PortalGun>(aCurrentWeapon))
	{
		FVector vStart = FirstPersonCameraComponent->GetComponentLocation();
		FVector vForwardVector = FirstPersonCameraComponent->GetForwardVector();
		FVector vEnd = (vStart + (vForwardVector * 5000.0f));

		FHitResult OutHitMain;

		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		//	DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Red, false, 0.01f);

		bool isHitMain = GetWorld()->LineTraceSingleByChannel(OutHitMain, vStart, vEnd, ECC_Visibility, CollisionParams);

		if (isHitMain)
		{
			if (OutHitMain.bBlockingHit)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *OutHitMain.GetActor()->GetName()));
				//UE_LOG(LogTemp, Warning, TEXT("Tag: %d"), OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")));
			}
		}

		if (OutHitMain.GetActor() != NULL)
		{
			if (OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")))
			{
				bCanFirePortal = true;

			}
			else if (!OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")))
			{
				bCanFirePortal = false;
			}
		}
	}
}

void AGEPortalCharacter::SetDefaultWeapon()
{
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.Owner = this;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	aSpawnedDefaultWeapon = GetWorld()->SpawnActor<AWeapon_AssaultRifle>(DefaultWeapon,
				FVector(0.0f, 0.0f, -1000.0f),FRotator(0.0f, 0.0f, 0.0f), ActorSpawnParams);

	
	//This only runs with the default weapon On Begin Play
	aCurrentWeapon = aSpawnedDefaultWeapon;	
	AddWeaponToPlayerInventory(aSpawnedDefaultWeapon);
		
}

void AGEPortalCharacter::ReloadWeapon()
{
	if (HasAuthority())
	{
		AWeaponBase* WeaponToReload = Cast<AWeaponBase>(aCurrentWeapon);
	
		if (WeaponToReload->WeaponBase_GetIsReloading())
		{
			return;
		}

		WeaponToReload->WeaponBase_Reload();
	}

	if (!HasAuthority())
	{
		AWeaponBase* WeaponToReload = Cast<AWeaponBase>(aCurrentWeapon);  //CHANGED HERE
		WeaponToReload->UpdateAmmoReloadOnClient(); //CHANGED HERE
		Server_OnRep_Reload();
	}

}

#pragma endregion Weapon


#pragma region WeaponSwitching

void AGEPortalCharacter::AddWeaponToPlayerInventory(AActor* aWeaponToAdd)
{
	UE_LOG(LogTemp, Warning, TEXT("ADDED WEAPON"));
	Weapons.Add(aWeaponToAdd);

	iCurrentWeapon = Weapons.Num()-1;

	ChangeCurrentWeapon(aWeaponToAdd);
	
	UE_LOG(LogTemp, Warning, TEXT("HIDDING"));
	aWeaponToAdd->SetActorHiddenInGame(true);

	
}

void AGEPortalCharacter::ChangeCurrentWeapon(AActor* WeaponToChangeTo)
{
	if (HasAuthority())
	{
		if (aCurrentWeapon != nullptr)
		{
			aCurrentWeapon = WeaponToChangeTo;
			Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
			FP_Weapon->SetMaterial(0, Cast<AWeaponBase>(aCurrentWeapon)->GetWeaponMesh()->GetMaterial(0));		
			Server_OnRep_TPWeaponMaterial(FP_Weapon->GetMaterial(0));
		}	
	}

	if (!HasAuthority())
	{
		aCurrentWeapon = WeaponToChangeTo;
		Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
		FP_Weapon->SetMaterial(0, Cast<AWeaponBase>(aCurrentWeapon)->GetWeaponMesh()->GetMaterial(0));
		Server_OnRep_TPWeaponMaterial(FP_Weapon->GetMaterial(0));
		Client_OnRep_ChangeWeapon(Cast<AActor>(aCurrentWeapon));
	}
	
}

void AGEPortalCharacter::CheckIfWeaponIsAlreadyCollected(AActor* ClassToCheck)
{
	if (Weapons.Num() > 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("Checking"));
		for (int i = 0; i < Weapons.Num(); ++i)
		{
			if (ClassToCheck->GetClass() == Weapons[i]->GetClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("Weapon Already Added"));
				bCanPickupWeapon = false;
			}
			else
			{
				bCanPickupWeapon = true;
			}
		}
	}
	else
	{
		bCanPickupWeapon = true;
	}
	
}

bool AGEPortalCharacter::CanPickupWeapon()
{
	return bCanPickupWeapon;
}

void AGEPortalCharacter::SwitchUpWeapon()
{
	if (HasAuthority())
	{
		if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
		{
			if (Weapons.Num() == 1)
			{
				return;
			}
			++iCurrentWeapon;
	
			if (iCurrentWeapon > Weapons.Num()-1)
			{
				iCurrentWeapon = 0;
			}
	 
			ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
		}
	}
	else
	{
		Server_OnRep_WeaponSwitchUp(aCurrentWeapon);
	}
		
}

void AGEPortalCharacter::SwitchDownWeapon()
{
	if (HasAuthority())
	{
		if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
		{
			--iCurrentWeapon;
			
			if (iCurrentWeapon < 0)
			{
				iCurrentWeapon = Weapons.Num()-1;
			}
		
			ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
		}
	}
	else
	{
		Server_OnRep_WeaponSwitchDown(aCurrentWeapon);
	}
}

void AGEPortalCharacter::AddHUDToViewport()
{
	if (wHUDClass == nullptr)
	{
		return;
	}

	wHUDWidget = CreateWidget<UUserWidget>(GetWorld(), wHUDClass);

	if (wHUDWidget == nullptr)
	{
		return;
	}

	wHUDWidget->AddToViewport();
}

TArray<AActor*> AGEPortalCharacter::GetWeapons()
{
	return Weapons;
}

void AGEPortalCharacter::ChooseWeaponToReceiveAmmo(int WeaponIndex, int AmountOfAmmo)
{
	Cast<AWeaponBase>(Weapons[WeaponIndex])->WeaponBase_AddAmmo(AmountOfAmmo);
}

#pragma endregion WeaponSwitching


#pragma region Healthsystem

void AGEPortalCharacter::DamagePlayer(int iDamageToTake, AActor* Killer)
{
	if (bIsPlayerDead || iCurrentHealth <= 0 || Killer == this || Killer == nullptr)
	{
		return;
	}
	
	if (iCurrentHealth > 0)
	{
		iCurrentHealth -= iDamageToTake;
		Client_OnRep_CurrentHealth(iCurrentHealth);
		
		if (iCurrentHealth <= 0)
		{
			iCurrentHealth = 0;
			
			Client_OnRep_CurrentHealth(iCurrentHealth);
			
			bIsPlayerDead = true;
			Server_Rep_IsPlayerDead(bIsPlayerDead);

			
			
			if (HasAuthority())
			{
				if (!Killer) return;
              	APawn* KillerPawn = Cast<APawn>(Killer);
                if (!KillerPawn) return;
                AController* KillerController = KillerPawn->GetController();
                			
                if (!KillerController) return;
                AGEPlayerState* KillerPlayerState = KillerController->GetPlayerState<AGEPlayerState>();
                			
                if (!KillerPlayerState) return;
                KillerPlayerState->AddKill();
                			
                if (!this->GetController()) return;
                AGEPlayerState* NoobPlayerState = this->GetController()->GetPlayerState<AGEPlayerState>();
                if (!NoobPlayerState) return;
                NoobPlayerState->AddDeath();

				NetMulticast_OnRep_DeathRagdoll();
			}
			else
			{
				Server_OnRep_ScoreUpdate(Killer);
			}
						
			FTimerHandle RespawnHandle;
			GetWorld()->GetTimerManager().SetTimer(RespawnHandle,this, &AGEPortalCharacter::Respawn, fRespawnTimer,false);

		}
	}
}

void AGEPortalCharacter::Client_OnRep_ScoreUpdate_Implementation(AActor* Killer)
{
	if (!Killer) return;
	APawn* KillerPawn = Cast<APawn>(Killer);
	if (!KillerPawn) return;
	AController* KillerController = KillerPawn->GetController();
			
	if (!KillerController) return;
	AGEPlayerState* KillerPlayerState = KillerController->GetPlayerState<AGEPlayerState>();
			
	if (!KillerPlayerState) return;
	KillerPlayerState->AddKill();
			
	if (!this->GetController()) return;
	AGEPlayerState* NoobPlayerState = this->GetController()->GetPlayerState<AGEPlayerState>();
	if (!NoobPlayerState) return;
	NoobPlayerState->AddDeath();
}

void AGEPortalCharacter::Server_OnRep_ScoreUpdate_Implementation(AActor* Killer)
{
	if (!Killer) return;
	APawn* KillerPawn = Cast<APawn>(Killer);
	if (!KillerPawn) return;
	AController* KillerController = KillerPawn->GetController();
                			
	if (!KillerController) return;
	AGEPlayerState* KillerPlayerState = KillerController->GetPlayerState<AGEPlayerState>();
                			
	if (!KillerPlayerState) return;
	KillerPlayerState->AddKill();
                			
	if (!this->GetController()) return;
	AGEPlayerState* NoobPlayerState = this->GetController()->GetPlayerState<AGEPlayerState>();
	if (!NoobPlayerState) return;
	NoobPlayerState->AddDeath();
	
	Client_OnRep_ScoreUpdate(Killer);
}

void AGEPortalCharacter::Respawn_Implementation()
{
	if (HasAuthority())
	{
		CheckIfHasPortalGun();
		AActor* CosmeticsHelper = UGameplayStatics::GetActorOfClass(GetWorld(),ACosmeticsHelper::StaticClass());
		Cast<ACosmeticsHelper>(CosmeticsHelper)->SaveCosmetics(this->GetMesh3P()->GetMaterial(0));
		
		AGEPortalGameMode* GameMode = (AGEPortalGameMode*)GetWorld()->GetAuthGameMode();
		if (!GetController()) return;
		if (!GameMode) return;
		
		GameMode->Spawn(this->GetController());
	}
}

void AGEPortalCharacter::CheckIfHasPortalGun()
{
	for (int i = 0; i < Weapons.Num(); ++i)
	{
		if (Cast<AWeapon_PortalGun>(Weapons[i]))
		{
			AActor* WeaponManager = UGameplayStatics::GetActorOfClass(GetWorld(),AWeaponsManager::StaticClass());
			Cast<AWeaponsManager>(WeaponManager)->SpawnPortalGun();
		
			break;
		}
	}
}

#pragma endregion HealthSystem


#pragma region Replication

void AGEPortalCharacter::Server_OnRep_TPWeaponMaterial_Implementation(UMaterialInterface* Material)
{
	NetMulticast_OnRep_TPWeaponMaterial(Material);
}

void AGEPortalCharacter::NetMulticast_OnRep_TPWeaponMaterial_Implementation(UMaterialInterface* Material)
{
	TP_Gun->SetMaterial(0, Material);
}

void AGEPortalCharacter::Server_OnRep_BodyColor_Implementation(UMaterialInterface* Material)
{
	NetMulticast_OnRep_BodyColor(Material);
}

void AGEPortalCharacter::NetMulticast_OnRep_BodyColor_Implementation(UMaterialInterface* Material)
{
	Mesh1P->SetMaterial(0,Material);
	Mesh3P->SetMaterial(0,Material);
}

void AGEPortalCharacter::Server_Rep_IsPlayerDead_Implementation(bool IsPlayerDead)
{
	bIsPlayerDead = IsPlayerDead;
}

void AGEPortalCharacter::Client_OnRep_CurrentHealth_Implementation(int Health)
{
	iCurrentHealth = Health;
}

void AGEPortalCharacter::Server_OnRep_OnFire_Implementation(FVector Start, FVector Forward, FVector End, AActor* Player)
{
	if(Cast<AWeapon_PortalGun>(aCurrentWeapon))
	{
		FirePortalGunBlue();
	}
	else
	{
		AWeaponBase * myWeapon = Cast<AWeaponBase>(aCurrentWeapon);
		if (myWeapon->WeaponBase_CheckFireRate())
		{
			return;
		}
		myWeapon->WeaponBase_StartFireRate();
		if (myWeapon->WeaponBase_CheckAmmo(aCurrentWeapon))
		{
			myWeapon->WeaponBase_Shoot(Start, Forward, End, Player);
		}
	}
	
	Client_OnRep_OnFire(Start, Forward, End, Player);
}

void AGEPortalCharacter::Client_OnRep_OnFire_Implementation(FVector Start, FVector Forward, FVector End, AActor* Player)
{
	if(Cast<AWeapon_PortalGun>(aCurrentWeapon))
	{
		FirePortalGunBlue();
	}
	else
	{
		AWeaponBase * myWeapon = Cast<AWeaponBase>(aCurrentWeapon);
		if (myWeapon->WeaponBase_CheckFireRate())
		{
			return;
		}
		myWeapon->WeaponBase_StartFireRate();
		if (myWeapon->WeaponBase_CheckAmmo(aCurrentWeapon))
		{
			myWeapon->WeaponBase_Shoot(Start, Forward, End, Player);
		}
	}
}

void AGEPortalCharacter::NetMulticast_OnRep_DeathRagdoll_Implementation()
{
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh3P()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh3P()->SetAllBodiesSimulatePhysics(true);
	this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGEPortalCharacter::Server_OnRep_Reload_Implementation()
{
	AWeaponBase* WeaponToReload = Cast<AWeaponBase>(aCurrentWeapon);

	if (WeaponToReload->WeaponBase_GetIsReloading())
	{
		return;
	}

	WeaponToReload->WeaponBase_Reload();
}

void AGEPortalCharacter::Server_OnRep_ChangeWeapon_Implementation(AActor* WeaponToChangeTo)
{
	if (aCurrentWeapon != nullptr)
	{
		aCurrentWeapon = WeaponToChangeTo;
		Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
		FP_Weapon->SetMaterial(0, Cast<AWeaponBase>(WeaponToChangeTo)->GetWeaponMesh()->GetMaterial(0));		
		Server_OnRep_TPWeaponMaterial(FP_Weapon->GetMaterial(0));
	}

	Client_OnRep_ChangeWeapon(WeaponToChangeTo);
}

void AGEPortalCharacter::Client_OnRep_ChangeWeapon_Implementation(AActor* WeaponToChangeTo)
{
	if (aCurrentWeapon != nullptr)
	{
		aCurrentWeapon = WeaponToChangeTo;
		Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
		FP_Weapon->SetMaterial(0, Cast<AWeaponBase>(WeaponToChangeTo)->GetWeaponMesh()->GetMaterial(0));		
		Server_OnRep_TPWeaponMaterial(FP_Weapon->GetMaterial(0));
	}	
}

void AGEPortalCharacter::Server_OnRep_WeaponSwitchUp_Implementation(AActor* WeaponToChangeTo)
{
	if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
	{
		if (Weapons.Num() == 1)
		{
			return;
		}
		++iCurrentWeapon;

		if (iCurrentWeapon > Weapons.Num()-1)
		{
			iCurrentWeapon = 0;
		}
	 
		ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
	}

	Client_OnRep_WeaponSwitchUp(WeaponToChangeTo);
}

void AGEPortalCharacter::Client_OnRep_WeaponSwitchUp_Implementation(AActor* WeaponToChangeTo)
{
	if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
	{
		if (Weapons.Num() == 1)
		{
			return;
		}
		++iCurrentWeapon;

		if (iCurrentWeapon > Weapons.Num()-1)
		{
			iCurrentWeapon = 0;
		}
	 
		ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
	}
}

void AGEPortalCharacter::Server_OnRep_WeaponSwitchDown_Implementation(AActor* WeaponToChangeTo)
{
	if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
	{
		--iCurrentWeapon;
		
		if (iCurrentWeapon < 0)
		{
			iCurrentWeapon = Weapons.Num()-1;
		}
		
		ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
	}
	
	Client_OnRep_WeaponSwitchDown(WeaponToChangeTo);
}

void AGEPortalCharacter::Client_OnRep_WeaponSwitchDown_Implementation(AActor* WeaponToChangeTo)
{
	if (!Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_GetIsReloading())
	{
		--iCurrentWeapon;
		
		if (iCurrentWeapon < 0)
		{
			iCurrentWeapon = Weapons.Num()-1;
		}
		
		ChangeCurrentWeapon(Weapons[iCurrentWeapon]);
	}
}

void AGEPortalCharacter::Server_OnRep_ShootOrangePortal_Implementation()
{
	if (bCanFirePortal)
	{
		if (Cast<AWeapon_PortalGun>(aCurrentWeapon))
		{
			Cast<AWeaponBase>(aCurrentWeapon)->WeaponBase_SetPlayer(this);
			Cast<AWeapon_PortalGun>(aCurrentWeapon)->PreFireGun();
			Cast<AWeapon_PortalGun>(aCurrentWeapon)->FirePortalProjectileOrange();
		}
	}
}


#pragma endregion Replication

