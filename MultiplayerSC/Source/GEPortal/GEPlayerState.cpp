// Fill out your copyright notice in the Description page of Project Settings.


#include "GEPlayerState.h"

void AGEPlayerState::AddKill()
{
	++iPlayerKills;
}

void AGEPlayerState::AddDeath()
{
	++iPlayerDeaths;
}

void AGEPlayerState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{ 
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGEPlayerState, iPlayerKills);
	DOREPLIFETIME(AGEPlayerState, iPlayerDeaths);
}