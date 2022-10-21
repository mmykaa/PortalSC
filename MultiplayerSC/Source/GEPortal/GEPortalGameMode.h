// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/GameModeBase.h"
#include "GEPortalGameMode.generated.h"

UCLASS(minimalapi)
class AGEPortalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGEPortalGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<UMaterialInterface*> Colors;

	TArray<AActor*>Players;
	int iMaxNumberOfPlayers;
	int iCurrentNumberOfPlayers;
	void OnPlayerLogin();
	APlayerController* NewPlayerToGiveColor;
	void CheckPlayerLogIn(APlayerController* NewPlayer);


	UPROPERTY() FVector DefaultSpawnLocation;
	UPROPERTY() FRotator DefaultSpawnRotation;
	UFUNCTION() void Spawn(AController* Controller);
	UPROPERTY() UMaterialInterface* SavedBodyMaterial;
	
};



