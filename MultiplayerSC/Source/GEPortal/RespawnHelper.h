// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnHelper.generated.h"

UCLASS()
class GEPORTAL_API ARespawnHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARespawnHelper();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere) TSubclassOf<APawn> DefaultPawnClass;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> SpawnPointsClass;
	TArray<class ASpawnPoint*> PossibleSpawnPoints;	
	UPROPERTY() FVector DefaultSpawnLocation;
	UPROPERTY() FRotator DefaultSpawnRotation;
	UPROPERTY() FTimerHandle RespawnHandle;
	UPROPERTY() float fRespawnTimer;
	UFUNCTION() void Spawn(AController* Controller, APawn* MyPawn);
	TArray<AController*> PlayersToRespawn;
	
	
	UFUNCTION() ASpawnPoint* GetSpawnPoint();
	UFUNCTION() void Respawn(AController* Controller);
	UPROPERTY() UMaterialInterface* SavedBodyMaterial;

	
};
