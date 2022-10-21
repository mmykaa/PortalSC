// Fill out your copyright notice in the Description page of Project Settings.


#include "CosmeticsHelper.h"

// Sets default values
ACosmeticsHelper::ACosmeticsHelper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACosmeticsHelper::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACosmeticsHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void  ACosmeticsHelper::SaveCosmetics(UMaterialInterface* BodyMaterial)
{
	for (int i = 0; i < Colors.Num(); ++i)
	{
		if (Colors.Find(BodyMaterial,i))
		{
			SavedBodyMaterial = Colors[i];
			break;
		}
	}
}

