// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoSpawnPoint.h"

// Sets default values
AAmmoSpawnPoint::AAmmoSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAmmoSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmoSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/** Nothing to see here */