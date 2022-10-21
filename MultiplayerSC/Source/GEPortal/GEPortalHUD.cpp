// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"


AGEPortalHUD::AGEPortalHUD()
{

}


void AGEPortalHUD::DrawHUD()
{
	Super::DrawHUD();


	if (!isCreated)
	{
		if (wCrosshairClass == nullptr)
		{
			return;
		}



		wCrossHairWidget = CreateWidget<UUserWidget>(GetWorld(), wCrosshairClass);



		if (wCrossHairWidget == nullptr)
		{
			return;
		}

		wCrossHairWidget->AddToViewport();

		isCreated = true;

	}

}
