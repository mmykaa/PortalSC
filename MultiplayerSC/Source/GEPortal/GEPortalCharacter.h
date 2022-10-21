// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GEPortalCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AGEPortalCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Mesh)
		USkeletalMeshComponent* Mesh1P;
	
	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		USkeletalMeshComponent* FP_Weapon;


	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* Mesh3P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USkeletalMeshComponent* TP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		USceneComponent* FP_MuzzleLocation;


	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* FirstPersonCameraComponent;

	
	AGEPortalCharacter();

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;
	
	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint8 bUsingMotionControllers : 1;

protected:
	bool bPlayerCanFire;
	void OnFire();

	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface


public:
	/** Returns Mesh1P subobject **/
	UFUNCTION(BlueprintCallable) USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns the 3P Mesh Subobject */
	UFUNCTION(BlueprintCallable)USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns the Current  */
	UFUNCTION(BlueprintCallable) AActor* GetCurrentWeapon() const { return aCurrentWeapon; }

	/** Disables the player input */
	UFUNCTION() void DisablePlayerInput();

	/** Enables the player Input */
	UFUNCTION() void EnablePlayerInput();

	/** Updates Cosmetics On Other Players After Login */
	void UpdateCosmeticsOnLogIn();

	/** Gets the gun offset */
	FVector GetGunOffset();
	
	USceneComponent* GetMuzzle();
	FRotator GetPlayerControlRotation();
	FTimerHandle DestroyHandle;


#pragma region PortalAiming
	
	UPROPERTY() FTimerHandle fthCheckForPortableWalls;
	UPROPERTY() bool bCanFirePortal;

	void CheckIfCanFirePortals();

#pragma endregion PortalAiming


#pragma region Weapons

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> DefaultWeapon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	TArray<AActor*> Weapons;
	UPROPERTY() AActor* aCurrentWeapon;
	UPROPERTY() AActor* aSpawnedDefaultWeapon;
	UPROPERTY() int iCurrentWeapon;
	UPROPERTY() bool bCanChangeWeapon;
	UPROPERTY() bool bCanPickupWeapon;

	
	UFUNCTION()	void AddWeaponToPlayerInventory(AActor* aWeaponToAdd);
	UFUNCTION()	void ChangeCurrentWeapon(AActor* WeaponToChangeTo);

	UFUNCTION() void CheckIfWeaponIsAlreadyCollected(AActor* ClassToCheck);
	UFUNCTION() bool CanPickupWeapon();
	UFUNCTION()	void FirePortalGunOrange();
	UFUNCTION()	void FirePortalGunBlue();
	UFUNCTION()	void SetDefaultWeapon();
	UFUNCTION() void ReloadWeapon();
	UFUNCTION()	void SwitchUpWeapon();
	UFUNCTION()	void SwitchDownWeapon();
	UFUNCTION() TArray<AActor*> GetWeapons();
	UFUNCTION() void ChooseWeaponToReceiveAmmo(int WeaponIndex, int AmountOfAmmo);
	UFUNCTION() void CheckIfHasPortalGun();

#pragma endregion Weapons


#pragma region Widgets

	UPROPERTY(EditDefaultsOnly, Category = "HUD", Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGEPortalHUD> HUDClass;

	/** Store the HUD UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wHUDClass;
	
	/** Stores the HUD widget */
	UPROPERTY()	class UUserWidget* wHUDWidget;

	//////////////////////////////////////////////////////////////////////////

	
	void AddHUDToViewport();
	
	UFUNCTION(BlueprintCallable) int CurrentHealthToUI() const { return iCurrentHealth; }

#pragma endregion Widgets


#pragma region Health System

	/** . */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	int iMaxHealth;

	/** . */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	int iCurrentHealth;
	
	/** . */
	UPROPERTY()	bool bIsPlayerDead;
	UPROPERTY()	bool bIsRespawning;
	UPROPERTY() bool bIsBodyHit;
	UPROPERTY() float fRespawnTimer;
	

	//////////////////////////////////////////////////////////////////////////
	
	/** Damage Calculations and Check if the player is dead or not */
	UFUNCTION()	void DamagePlayer(int iDamageToTake, AActor* Killer);
	
	/** returns the MaxHealth so an Headshot can always kill the player" */
	UFUNCTION()	int GetMaxHealth() const{ return iMaxHealth;}

	
	/** returns the Current Health " */
	UFUNCTION()	int GetCurrentHealth() const{ return iCurrentHealth;}
	
	UFUNCTION()	bool GetIsPlayerDead() const{ return bIsPlayerDead;}

UFUNCTION(Server, Reliable) void Respawn();

#pragma endregion Health System


#pragma region Replication

	UFUNCTION() bool GetPlayerAuthority() const { return HasAuthority(); }

	UFUNCTION(Server, Reliable) void Server_OnRep_TPWeaponMaterial(UMaterialInterface* Material);
	UFUNCTION(NetMulticast, Reliable) void NetMulticast_OnRep_TPWeaponMaterial(UMaterialInterface* Material);

	UFUNCTION(Server, Reliable) void Server_OnRep_BodyColor(UMaterialInterface* Material);
	UFUNCTION(NetMulticast, Reliable) void NetMulticast_OnRep_BodyColor(UMaterialInterface* Material);
	
	UFUNCTION(Server, Reliable)	void Server_Rep_IsPlayerDead(bool IsPlayerDead);
	
	UFUNCTION(Client, Reliable) void Client_OnRep_CurrentHealth(int Health);

	UFUNCTION(Server, Reliable) void Server_OnRep_OnFire(FVector Start, FVector Forward, FVector End, AActor* Player);
	UFUNCTION(Client, Reliable) void Client_OnRep_OnFire(FVector Start, FVector Forward, FVector End, AActor* Player);

	UFUNCTION(Server, Reliable) void Server_OnRep_ShootOrangePortal();
	UFUNCTION(Server, Reliable) void Server_OnRep_Reload();
	
	UFUNCTION(NetMulticast, Reliable) void NetMulticast_OnRep_DeathRagdoll();

	UFUNCTION(Server, Reliable) void Server_OnRep_ChangeWeapon(AActor * WeaponToChangeTo);
	UFUNCTION(Client, Reliable) void Client_OnRep_ChangeWeapon(AActor * WeaponToChangeTo);

	UFUNCTION(Server, Reliable) void Server_OnRep_WeaponSwitchUp(AActor * WeaponToChangeTo);
	UFUNCTION(Client, Reliable) void Client_OnRep_WeaponSwitchUp(AActor * WeaponToChangeTo);
	UFUNCTION(Server, Reliable) void Server_OnRep_WeaponSwitchDown(AActor * WeaponToChangeTo);
	UFUNCTION(Client, Reliable) void Client_OnRep_WeaponSwitchDown(AActor * WeaponToChangeTo);

	UFUNCTION(Server, Reliable) void Server_OnRep_ScoreUpdate(AActor * Killer);
	UFUNCTION(Client, Reliable) void Client_OnRep_ScoreUpdate(AActor * Killer);




#pragma endregion Replication
	
};

