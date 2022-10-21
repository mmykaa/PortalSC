// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GEPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API AGEPlayerState : public APlayerState
{
	GENERATED_BODY()

	/** Stores the player total kills */
	UPROPERTY(Replicated) int iPlayerKills;

	/** Stores the player total deaths */
	UPROPERTY(Replicated) int iPlayerDeaths;
	
public:
	
	/** Increments a kill */
	UFUNCTION() void AddKill();

	/** Increments a death */
	UFUNCTION() void AddDeath();

	/** Returns the Kills to the UI */
	UFUNCTION(BlueprintCallable, BlueprintPure) int GetKills()  { return  iPlayerKills;}
	
	/** Returns the Deaths to the UI */
	UFUNCTION(BlueprintCallable, BlueprintPure) int GetDeaths()  { return  iPlayerDeaths;}

	/** Replicates our K/D */
	void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;
};
