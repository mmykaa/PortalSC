// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystem.h"
#include "Engine/GameInstance.h"
#include "Interfaces//OnlineSessionInterface.h"
#include "SessionsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API USessionsSubsystem : public UGameInstance
{
	GENERATED_BODY()

public:
	USessionsSubsystem();

protected:

	IOnlineSessionPtr SessionInterface;
	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);
	virtual void OnFindSessionComplete(bool Succeeded);
	virtual void OnFindSessionComplete(FName SessionName,EOnJoinSessionCompleteResult::Type Result);


	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	UFUNCTION(BlueprintCallable) void CreateServer();
	UFUNCTION(BlueprintCallable) void JoinServer();

};
