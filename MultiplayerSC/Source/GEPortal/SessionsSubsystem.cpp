// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionsSubsystem.h"
#include "Engine/World.h"
#include "OnlineSubsystem.h"
#include  "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

USessionsSubsystem::USessionsSubsystem()
{
}

void USessionsSubsystem::Init()
{
	Super::Init();

	//Access OnlineSubsystem
    	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get())
    	{
    		//Access Session Interface
    		SessionInterface = SubSystem->GetSessionInterface();

    		if(SessionInterface.IsValid())
    		{
   				SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionsSubsystem::OnCreateSessionComplete);
    			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USessionsSubsystem::OnFindSessionComplete);
    			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USessionsSubsystem::OnFindSessionComplete);

    		}
    	}
}

void USessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("Create Session Success: %d"), Succeeded);

	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/Maps/FirstPersonExampleMap?listen"); 
	}
}

void USessionsSubsystem::OnFindSessionComplete(bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("Session Found: %d"), Succeeded);

	if (Succeeded)
	{
		
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		UE_LOG(LogTemp, Warning, TEXT("Servers Found: %i"), SearchResults.Num());

		if (SearchResults.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("Joining Server"));

			SessionInterface->JoinSession(0, "My Session", SearchResults[0]);
		}
		
	}
}

void USessionsSubsystem::OnFindSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("Joined"));
	if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);
		if (JoinAddress != "")
		{
			PController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

void USessionsSubsystem::CreateServer()
{
	UE_LOG(LogTemp, Warning, TEXT("Server Creating"));
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 4;
	
	SessionInterface->CreateSession(0, FName("My Session"), SessionSettings);
}

void USessionsSubsystem::JoinServer()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 1000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}
