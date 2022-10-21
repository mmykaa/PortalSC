// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/SceneComponent.h"
#include "AmmoBase.generated.h"


UENUM(BlueprintType)
enum EAmmoType
{
	AmmoAssaultRifle     UMETA(DisplayName = "AssaultRifle"),
	AmmoShotgun			 UMETA(DisplayName = "Shotgun"),
	AmmoRocketLauncher   UMETA(DisplayName = "RocketLauncher"),
};

UCLASS()
class GEPORTAL_API AAmmoBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TEnumAsByte<EAmmoType> AmmoType;

	/** Stores the ammo amount that will be givven to the player */
	UPROPERTY(BlueprintReadWrite,EditAnywhere) int iAmmoAmount;

	/** Object's Root */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USceneComponent*  DefaultRoot;

	/** Main Sphere Collider */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USphereComponent * SphereCollider;

	/** Prevents the the overlap fidding another player */
	UPROPERTY() bool bFoundActor;

	/** Reference to the player that will receive the ammo */
	UPROPERTY() AGEPortalCharacter* MyPlayer;


	/////////////////////////////////////////////////////////////////////

	/** Gets his ammo type */
	UFUNCTION() void AmmoBase_GetAmmoType();

	/** Delivers Ammo to the weapon that the player has */ 
	UFUNCTION() void AmmoBase_DeliverAmmo(int WeaponIndex);

	/** Handles the Overlapping events */
	UFUNCTION() void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
