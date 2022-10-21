// Fill out your copyright notice in the Description page of Project Settings.


#include "RespawnHelper.h"

#include "EngineUtils.h"
#include "CosmeticsHelper.h"
#include "GEPortalCharacter.h"
#include "SpawnPoint.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ARespawnHelper::ARespawnHelper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	DefaultSpawnLocation = FVector(300.0f, 500.0f, 200.0f);
	DefaultSpawnRotation = FRotator::ZeroRotator;

	fRespawnTimer = 1.0f;
}

// Called when the game starts or when spawned
void ARespawnHelper::BeginPlay()
{
	Super::BeginPlay();

	UClass * SpawnPointClass = ASpawnPoint::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(),SpawnPointClass); Actor; ++Actor )
	{
		PossibleSpawnPoints.Add(Cast<ASpawnPoint>(*Actor));
	}
}

// Called every frame
void ARespawnHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



ASpawnPoint* ARespawnHelper::GetSpawnPoint()
{
	for (int32 i = 0; i < PossibleSpawnPoints.Num(); ++i)
	{
		int32 Slot = FMath::RandRange(0, PossibleSpawnPoints.Num()-1);

		if (PossibleSpawnPoints[Slot])
		{
			return PossibleSpawnPoints[Slot];
		}
	}
	
	return nullptr;
}


void ARespawnHelper::Spawn(AController* Controller, APawn* MyPawn)
{
	if (Controller)
	{
		if(ASpawnPoint * SpawnPoint = GetSpawnPoint())
		{
			FVector Location = 	SpawnPoint->GetActorLocation();
			FRotator Rotation = SpawnPoint->GetActorRotation();
		
		if (APawn * Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location,Rotation))
			{
				Controller->Possess(Pawn);
			}	
		}
		else
		{
			FVector Location = 	DefaultSpawnLocation;
			FRotator Rotation = DefaultSpawnRotation;
		
			if (APawn * Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location,Rotation))
			{
				Controller->Possess(Pawn);
			}	
		}
		AActor* CosmeticsHelper = UGameplayStatics::GetActorOfClass(GetWorld(),ACosmeticsHelper::StaticClass());
		SavedBodyMaterial = Cast<ACosmeticsHelper>(CosmeticsHelper)->GetSavedBodyMaterial();
	
		Cast<AGEPortalCharacter>(Controller->GetPawn())->GetMesh1P()->SetMaterial(0, SavedBodyMaterial);
		Cast<AGEPortalCharacter>(Controller->GetPawn())->GetMesh3P()->SetMaterial(0, SavedBodyMaterial);
		Cast<AGEPortalCharacter>(Controller->GetPawn())->UpdateCosmeticsOnLogIn();

		MyPawn->Destroy();
	}
}


void ARespawnHelper::Respawn(AController* Controller)
{
//	PlayersToRespawn.Add(Controller);
	
	
	FTimerDelegate RespawnDele;
	RespawnDele.BindUFunction(this, FName("Spawn"), Controller);
	GetWorld()->GetTimerManager().SetTimer(RespawnHandle, RespawnDele, 0.0f, false);
	

}


