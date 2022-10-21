// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"
#include "GEPortalGameMode.h"
#include "GEPortalCharacter.h"

#include "DrawDebugHelpers.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>

// Sets default values
APortal::APortal()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComponent);

	//Portal Border
	BorderMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("BorderMeshComponent"));
	BorderMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BorderMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	BorderMeshComponent->SetupAttachment(RootComponent);

	//Portal
	PortalMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("PortalMeshComponent"));
	PortalMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalMeshComponent->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	PortalMeshComponent->SetupAttachment(RootComponent);



	//Trigger
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
	BoxComponent->SetBoxExtent(FVector(50, 90, 125));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnOverlapBegin);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &APortal::OnOverlapEnd);
	BoxComponent->SetupAttachment(RootComponent);

	//Capture
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(FName("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(RootComponent);

	FPostProcessSettings CaptureSettings;

	CaptureSettings.bOverride_AmbientOcclusionQuality = true;
	CaptureSettings.AmbientOcclusionQuality = 0.0f;

	CaptureSettings.bOverride_MotionBlurAmount = true;
	CaptureSettings.MotionBlurAmount = 0.0f;

	CaptureSettings.bOverride_SceneFringeIntensity = true;
	CaptureSettings.SceneFringeIntensity = 0.0f;

//Deprecated in 5.0
//	CaptureSettings.bOverride_GrainIntensity = true;
//	CaptureSettings.GrainIntensity = 0.0f;

	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;
	CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;

//Deprecated in 5.0
//	CaptureSettings.bOverride_ScreenPercentage = true;
//	CaptureSettings.ScreenPercentage = 100.0f;

	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->bEnableClipPlane = true;
	SceneCaptureComponent->bOverride_CustomNearClippingPlane = true;
	SceneCaptureComponent->PostProcessSettings = CaptureSettings;

}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdatePortalSurfaces();
}

void APortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AGEPortalCharacter>(OtherActor))
	{
		Overlapping = true;

		GetWorld()->GetTimerManager().SetTimer(fthCheckIfCanTeleport, this, &APortal::CheckIfCanTeleport, 0.001f, true, 0.0f);

		if (PortalSurface != nullptr)
		{
			PortalSurface->SetActorEnableCollision(false);
		}
	}
}

void APortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AGEPortalCharacter>(OtherActor))
	{
		Overlapping = false;

		GetWorld()->GetTimerManager().ClearTimer(fthCheckIfCanTeleport);

		if (PortalSurface != nullptr)
		{
			PortalSurface->SetActorEnableCollision(true);
		}
	}
}

void APortal::Teleport()
{
	if (Target != NULL)
	{

		AGEPortalGameMode* PortalGameMode = GetWorld()->GetAuthGameMode<AGEPortalGameMode>();

		AGEPortalCharacter* PlayerCharacter = PortalGameMode->GetExistingPlayer();

		if (PlayerCharacter == NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("PLAYER CLASS NULL CANNOT TP"));
			return;
		}

		if (PlayerCharacter != NULL)
		{
			if (PortalGameMode->CheckIfThePlayerIsCrossingThePortal(PlayerCharacter->GetActorLocation(), GetActorLocation(), GetActorForwardVector(), bLastInFront, LastPosition))
			{
				FVector SavedVelocity = PlayerCharacter->GetCharacterMovement()->Velocity;

				FHitResult HitResult;

				FVector Location = PortalGameMode->ConvertLocation(PlayerCharacter->GetActorLocation(), this, Target);
				FRotator Rotation = PortalGameMode->ConvertRotation(PlayerCharacter->GetActorRotation(), this, Target);


				//Set the player Location and Rotation when Teleporting to the portal
				PlayerCharacter->SetActorLocationAndRotation(Location, Rotation, false, &HitResult, ETeleportType::TeleportPhysics);

				//Adjust the player Control rotation in so he faces the same direction and with the same angle that he enters in the portal
				//since we are in single player, 0 returns our player
				UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(PortalGameMode->ConvertRotation(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetControlRotation(), this, Target));

				//Set the new player velocity
				float DotForward = FVector::DotProduct(SavedVelocity, GetActorForwardVector());
				float DotRight = FVector::DotProduct(SavedVelocity, GetActorRightVector());
				float DotUp = FVector::DotProduct(SavedVelocity, GetActorUpVector());

				FVector PortalTravellingVelocity = DotForward * Target->GetActorForwardVector() + DotRight * Target->GetActorRightVector() + DotUp * Target->GetActorUpVector();

				//Invert the velocity to maintain the player movement
				PlayerCharacter->GetCharacterMovement()->Velocity = -PortalTravellingVelocity;

				LastPosition = Location;

				Overlapping = false;
			}
		}
	}
}

void APortal::CheckIfCanTeleport()
{
	if (Overlapping)
	{
		Teleport();
	}
}

APortal* APortal::GetTarget() const
{
	return Target;
}

void APortal::SetTarget(APortal* NewTarget)
{
	if (NewTarget != nullptr)
	{
		Target = NewTarget;
	}
}

AActor* APortal::GetPortalSurface() const
{
	return PortalSurface;
}

void APortal::SetPortalSurface(AActor* Surface)
{
	PortalSurface = Surface;
}


void APortal::UpdatePortalSurfaces()
{
	AGEPortalGameMode* PortalGameMode = GetWorld()->GetAuthGameMode<AGEPortalGameMode>();

	if (GetWorld() != NULL)
	{
		APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));

		if (CameraManager != NULL)
		{
			if (Target != NULL)
			{
				FVector Position = PortalGameMode->ConvertLocation(CameraManager->GetCameraLocation(), this, Target);
 				FRotator Rotation = PortalGameMode->ConvertRotation(CameraManager->GetCameraRotation(), this, Target);
				
				Target->SceneCaptureComponent->SetWorldLocationAndRotation(Position, Rotation);
		
				//Distance beetween the camera and the portal so it does not render the objects behind the exit portal.
				float dynamicClipingPlane = sqrt((pow(SceneCaptureComponent->GetComponentLocation().X - GetActorLocation().X, 2))
					+ (pow(SceneCaptureComponent->GetComponentLocation().Y - GetActorLocation().Y, 2))         //                  ________________________
					+ (pow(SceneCaptureComponent->GetComponentLocation().Z - GetActorLocation().Z, 2)));      //     Distance =  _/(x-x)^2+(y-y)^2+(z-z)^2

				SceneCaptureComponent->CustomNearClippingPlane = dynamicClipingPlane;


			}

		}
	}

}
