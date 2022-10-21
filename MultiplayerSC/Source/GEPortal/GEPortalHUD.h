// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GEPortalHUD.generated.h"

UCLASS()
class AGEPortalHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGEPortalHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;
	

	bool isCreated;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true")) TSubclassOf<class UUserWidget> wCrosshairClass;

	/** Stores the Crosshair widget */
	UPROPERTY() class UUserWidget* wCrossHairWidget;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;


};

