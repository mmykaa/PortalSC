// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalCharacter.h"
#include "GEBluePortalProjectile.h"
#include "GEOrangePortalProjectile.h"
#include "GEPortalGameMode.h"
#include "GEPortalHUD.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


AGEPortalCharacter::AGEPortalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(15.0f, 70.0f);
	
	// set our turn rates for input
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(RootComponent);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	TP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TP_Gun"));

	TP_Gun->SetOwnerNoSee(true);			// otherwise won't be visible in the multiplayer
	TP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

void AGEPortalCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	TP_Gun->AttachToComponent(Mesh3P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	Mesh1P->SetHiddenInGame(false, true);

	AGEPortalGameMode* PortalGameMode = GetWorld()->GetAuthGameMode<AGEPortalGameMode>();
	PortalGameMode->SetPlayer(this);

	GetWorld()->GetTimerManager().SetTimer(fthCheckForPortableWalls, this, &AGEPortalCharacter::CheckIfCanFirePortals, 0.001f, true, 0.0f);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AGEPortalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGEPortalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGEPortalCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGEPortalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGEPortalCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("FireBluePortalProjectile", IE_Pressed, this, &AGEPortalCharacter::FirePortalProjectileBlue);
	PlayerInputComponent->BindAction("FireOrangePortalProjectile", IE_Pressed, this, &AGEPortalCharacter::FirePortalProjectileOrange);
}


void AGEPortalCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGEPortalCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGEPortalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGEPortalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}


void AGEPortalCharacter::FirePortalProjectileBlue()
{
	if (bCanFirePortal)
	{
		if (aProjectileBluePortal != nullptr)
		{

			UWorld* const World = GetWorld();
			
			if (World != nullptr)
			{
				AGEPortalCharacter::OnFire();

				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<AGEBluePortalProjectile>(aProjectileBluePortal, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}
}

void AGEPortalCharacter::FirePortalProjectileOrange()
{
	if (bCanFirePortal)
	{
		// try and fire a projectile
		if (aProjectileOrangePortal != nullptr)
		{
			UWorld* const World = GetWorld();
			if (World != nullptr)
			{
				AGEPortalCharacter::OnFire();

				const FRotator SpawnRotation = GetControlRotation();

				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				// spawn the projectile at the muzzle
				World->SpawnActor<AGEOrangePortalProjectile>(aProjectileOrangePortal, SpawnLocation, SpawnRotation, ActorSpawnParams);

			}
		}

	}

}
void AGEPortalCharacter::OnFire()
{
	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}


void AGEPortalCharacter::CheckIfCanFirePortals()
{
	FVector vStart = FirstPersonCameraComponent->GetComponentLocation();
	FVector vForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector vEnd = (vStart + (vForwardVector * 5000.0f));

	FHitResult OutHitMain;

	FCollisionQueryParams CollisionParams;

//	DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Red, false, 0.01f);

	bool isHitMain = GetWorld()->LineTraceSingleByChannel(OutHitMain, vStart, vEnd, ECC_Visibility, CollisionParams);

	if (isHitMain)
	{
		if (OutHitMain.bBlockingHit)
		{
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *OutHitMain.GetActor()->GetName()));
				//UE_LOG(LogTemp, Warning, TEXT("Tag: %d"), OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")));
		}
	}

	if (OutHitMain.GetActor() != NULL)
	{
		if (OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")))
		{
			bCanFirePortal = true;

			AGEPortalHUD* myHUDClass = Cast<AGEPortalHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
			myHUDClass->UnFadeCrosshair();
		}
		else if (!OutHitMain.GetActor()->ActorHasTag(TEXT("PortableWall")))
		{
			bCanFirePortal = false;
			AGEPortalHUD* myHUDClass = Cast<AGEPortalHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
			myHUDClass->FadeCrosshair();
		}
	}


}
