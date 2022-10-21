// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalGameMode.h"

#include "AmmoBase.h"
#include "CosmeticsHelper.h"
#include "GEPortalHUD.h"
#include "GEPortalCharacter.h"

#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

#include "SpawnPoint.h"

#include "Kismet/GameplayStatics.h"

#include "TimerManager.h"


AGEPortalGameMode::AGEPortalGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Players/BP_PlayerA"));


	// set default pawn class to our Blueprinted character
	//DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGEPortalHUD::StaticClass();

	DefaultSpawnLocation = FVector(300.0f, 500.0f, 200.0f);
	DefaultSpawnRotation = FRotator::ZeroRotator;

	iMaxNumberOfPlayers = 4;
	iCurrentNumberOfPlayers = 0;
}

void AGEPortalGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AController* Controller = Cast<AController>(NewPlayer))
	{
		++iCurrentNumberOfPlayers;
		NewPlayerToGiveColor = NewPlayer;
		Players.Add(NewPlayer->GetPawn());
		Cast<AGEPortalCharacter>(NewPlayer->GetPawn())->DisablePlayerInput();
		
		if (iCurrentNumberOfPlayers == iMaxNumberOfPlayers)
		{
			FTimerHandle CheckPlayerLogIn;
			GetWorld()->GetTimerManager().SetTimer(CheckPlayerLogIn, this, &AGEPortalGameMode::OnPlayerLogin,1.0f, false, 1.0f);
		}
	}
}


void AGEPortalGameMode::OnPlayerLogin()
{
	CheckPlayerLogIn(NewPlayerToGiveColor);
}


void AGEPortalGameMode::CheckPlayerLogIn(APlayerController* NewPlayer)
{
	if (Players.Num() == iMaxNumberOfPlayers)
	{
		for (int i = 0; i < iMaxNumberOfPlayers; ++i)
		{
			Cast<AGEPortalCharacter>(Players[i])->GetMesh1P()->SetMaterial(0,Colors[i]);
			Cast<AGEPortalCharacter>(Players[i])->GetMesh3P()->SetMaterial(0,Colors[i]);
			Cast<AGEPortalCharacter>(Players[i])->UpdateCosmeticsOnLogIn();
			Cast<AGEPortalCharacter>(Players[i])->EnablePlayerInput();
			
			UE_LOG(LogTemp, Warning, TEXT("INPUT ENABLED"));
		}
	}
	if (Players.Num() > iMaxNumberOfPlayers)
	{
		NewPlayer->Destroy();
	}
}


void AGEPortalGameMode::BeginPlay()
{
	Super::BeginPlay();
}


void AGEPortalGameMode::Spawn(AController* Controller)
{
	if (!Controller) return;
	if (!Controller->GetPawn()) return;

	APawn* Pawn = Controller->GetPawn();
	Pawn->Reset();
	Pawn->Destroy();
		
	RestartPlayer(Controller);
				
	AActor* CosmeticsHelper = UGameplayStatics::GetActorOfClass(GetWorld(),ACosmeticsHelper::StaticClass());
	SavedBodyMaterial = Cast<ACosmeticsHelper>(CosmeticsHelper)->GetSavedBodyMaterial();

	AGEPortalCharacter * MyCharacter =  Cast<AGEPortalCharacter>(Controller->GetPawn());
	
	MyCharacter->GetMesh1P()->SetMaterial(0, SavedBodyMaterial);
	MyCharacter->GetMesh3P()->SetMaterial(0, SavedBodyMaterial);
	MyCharacter->UpdateCosmeticsOnLogIn();
}
