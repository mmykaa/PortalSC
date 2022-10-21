// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosmeticsHelper.generated.h"

UCLASS()
class GEPORTAL_API ACosmeticsHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACosmeticsHelper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Stores the Material of the player that died */
	UPROPERTY() UMaterialInterface* SavedBodyMaterial;

	/** Stores all the body materials so we can compare them */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<UMaterialInterface*> Colors;


	//////////////////////////////////////////////////////////

	
	/** Saves the cosmetics when the player dies so we can apply them after the respawning */
	UFUNCTION() void SaveCosmetics(UMaterialInterface* BodyMaterial);

	/** Returns the saved material before the players death */
	UFUNCTION() UMaterialInterface* GetSavedBodyMaterial() const {return SavedBodyMaterial;}

};
