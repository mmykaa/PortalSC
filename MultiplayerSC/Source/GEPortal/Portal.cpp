// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"
#include "GEPortalGameMode.h"
#include "GEPortalCharacter.h"

#include "DrawDebugHelpers.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <Kismet/KismetMathLibrary.h>

#include "MeshAttributes.h"
#include "RocketProjectile.h"
#include "WeaponBase.h"
#include "Weapon_AssaultRifle.h"
#include "Weapon_Shotgun.h"
#include "GameFramework/ProjectileMovementComponent.h"

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

//Deprecated
//	CaptureSettings.bOverride_GrainIntensity = true;
//	CaptureSettings.GrainIntensity = 0.0f;

	CaptureSettings.bOverride_ScreenSpaceReflectionQuality = true;
	CaptureSettings.ScreenSpaceReflectionQuality = 0.0f;

//Deprecated
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
		if (Cast<AWeaponBase>(OtherActor) || Cast<APortal>(OtherActor))
		{
			return;
		}
	
		if (Cast<AGEPortalCharacter>(OtherActor) && !bFoundActor)
		{
			bFoundActor = true;
			ObjectOverlapping = OtherActor;
			ObjectToTeleport = ObjectOverlapping;
			
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
	if (Cast<AWeaponBase>(OtherActor) || Cast<APortal>(OtherActor))
	{
		return;
	}
	
	if (Cast<AGEPortalCharacter>(OtherActor))
	{
		Overlapping = false;
		bFoundActor = false;

		GetWorld()->GetTimerManager().ClearTimer(fthCheckIfCanTeleport);

		if (PortalSurface != nullptr)
		{
			PortalSurface->SetActorEnableCollision(true);
		}
	}
		
		
}

void APortal::Teleport()
{
	if (Target != nullptr)
	{
		bIsTeleporting = true;
		
			if (CheckIfThePlayerIsCrossingThePortal(ObjectToTeleport->GetActorLocation(), GetActorLocation(), GetActorForwardVector(), bLastInFront, LastPosition))
			{
				
				if (Cast<AGEPortalCharacter>(ObjectToTeleport))
				{
					FVector SavedVelocity = Cast<AGEPortalCharacter>(ObjectToTeleport)->GetCharacterMovement()->Velocity;
				
					FHitResult HitResult;

					FVector Location = ConvertLocation(ObjectToTeleport->GetActorLocation(), this, Target);
					FRotator Rotation = ConvertRotation(ObjectToTeleport->GetActorRotation(), this, Target);


					//Set the player Location and Rotation when Teleporting to the portal
					ObjectToTeleport->SetActorLocationAndRotation(Location, Rotation, false, &HitResult, ETeleportType::TeleportPhysics);

					//Adjust the player Control rotation in so he faces the same direction and with the same angle that he enters in the portal
					Cast<AGEPortalCharacter>(ObjectToTeleport)->GetController()->SetControlRotation(ConvertRotation(Cast<AGEPortalCharacter>(ObjectToTeleport)->GetController()->GetControlRotation(), this, Target));
					
					//Set the new player velocity
					float DotForward = FVector::DotProduct(SavedVelocity, GetActorForwardVector());
					float DotRight = FVector::DotProduct(SavedVelocity, GetActorRightVector());
					float DotUp = FVector::DotProduct(SavedVelocity, GetActorUpVector());

					FVector PortalTravellingVelocity = DotForward * Target->GetActorForwardVector() + DotRight * Target->GetActorRightVector() + DotUp * Target->GetActorUpVector();

					//Invert the velocity to maintain the player movement
					 Cast<AGEPortalCharacter>(ObjectToTeleport)->GetCharacterMovement()->Velocity = -PortalTravellingVelocity;

					LastPosition = Location;

					Overlapping = false;
					bIsTeleporting = false;
				}
			}
	}
}

void APortal::CheckIfCanTeleport()
{
	if (Overlapping && ObjectToTeleport == ObjectOverlapping)
	{
		Teleport();
	}
}

void APortal::RedirectShot(FHitResult OutHit, FRotator OnShootPlayerRotation, AActor* Shooter)
{
	
	if (Target != nullptr)
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* InstatiatedRedirectHelper = GetWorld()->SpawnActor<AActor>(RedirectHelper, OutHit.Location,  OnShootPlayerRotation, ActorSpawnParams);
		
		FVector Position = ConvertLocation(InstatiatedRedirectHelper->GetActorLocation(), this, Target);
		FRotator Rotation = ConvertRotation(InstatiatedRedirectHelper->GetActorRotation(), this, Target);
		
		InstatiatedRedirectHelper->SetActorRelativeLocation(Position);
		InstatiatedRedirectHelper->SetActorRotation(Rotation + Cast<AGEPortalCharacter>(Shooter)->GetController()->GetControlRotation());
		
		FCollisionQueryParams CollisionParams;
	
		GetWorld()->LineTraceSingleByChannel(OutHit, InstatiatedRedirectHelper->GetActorLocation(),
			InstatiatedRedirectHelper->GetActorLocation() + (InstatiatedRedirectHelper->GetActorForwardVector() * 5000.0f), ECC_Visibility, CollisionParams);

		DrawDebugLine(GetWorld(), InstatiatedRedirectHelper->GetActorLocation(),
			(InstatiatedRedirectHelper->GetActorLocation() + (InstatiatedRedirectHelper->GetActorForwardVector() * 5000.0f)),
			FColor::Purple,false, 0.2f);

		if (Cast<AGEPortalCharacter>(OutHit.GetActor()))
		{
			if (Cast<AWeapon_AssaultRifle>(Cast<AGEPortalCharacter>(Shooter)->GetCurrentWeapon()))
			{
				AGEPortalCharacter* PlayerToDamage = Cast<AGEPortalCharacter>(OutHit.GetActor());
			
				if (OutHit.BoneName.ToString() == FString("head"))
				{
					PlayerToDamage->DamagePlayer(PlayerToDamage->GetMaxHealth(), Shooter);
				}
				else
				{
					int iDamageOnHit = Cast<AWeapon_AssaultRifle>(Cast<AGEPortalCharacter>(Shooter)->GetCurrentWeapon())->GetDamageOnHit();
					PlayerToDamage->DamagePlayer(iDamageOnHit, Shooter);
				}
			}

			if (Cast<AWeapon_Shotgun>(Cast<AGEPortalCharacter>(Shooter)->GetCurrentWeapon()))
			{
				AGEPortalCharacter* PlayerToDamage = Cast<AGEPortalCharacter>(OutHit.GetActor());
								
				int iDamageOnHit = Cast<AWeapon_Shotgun>(Cast<AGEPortalCharacter>(Shooter)->GetCurrentWeapon())->GetDamageOnHit();
				PlayerToDamage->DamagePlayer(iDamageOnHit, Shooter);
				
			}
		}
		
		InstatiatedRedirectHelper->Destroy();
	}
}


void APortal::RedirectProjectile(AActor* Projectile, FVector Location, FRotator OnShootPlayerRotation, AGEPortalCharacter* Shooter)
{
	if (Target != nullptr && Shooter != nullptr)
	{
		FVector SavedVelocity = Cast<ARocketProjectile>(Projectile)->GetProjectileMovement()->Velocity;

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* InstatiatedRedirectHelper = GetWorld()->SpawnActor<AActor>(RedirectHelper, Location,  OnShootPlayerRotation, ActorSpawnParams);
		
		FVector Position = ConvertLocation(InstatiatedRedirectHelper->GetActorLocation(), this, Target);
		FRotator Rotation = ConvertRotation(InstatiatedRedirectHelper->GetActorRotation(), this, Target);
	
		InstatiatedRedirectHelper->SetActorRelativeLocation(Position);
		InstatiatedRedirectHelper->SetActorRotation(Rotation + Cast<AGEPortalCharacter>(Shooter)->GetController()->GetControlRotation());

		Projectile->SetActorRelativeLocation(InstatiatedRedirectHelper->GetActorLocation());
		Projectile->SetActorRotation(InstatiatedRedirectHelper->GetActorRotation());
		
		//Set the new Projectile velocity
		float DotForward = FVector::DotProduct(SavedVelocity, GetActorForwardVector());
		float DotRight = FVector::DotProduct(SavedVelocity, GetActorRightVector());
		float DotUp = FVector::DotProduct(SavedVelocity, GetActorUpVector());

		FVector PortalTravellingVelocity = DotForward * Target->GetActorForwardVector() + DotRight * Target->GetActorRightVector() + DotUp * Target->GetActorUpVector();

		//Invert the velocity to maintain the player movement
		Cast<ARocketProjectile>(Projectile)->GetProjectileMovement()->Velocity = -PortalTravellingVelocity;

		InstatiatedRedirectHelper->Destroy();
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
	if (GetWorld() != NULL)
	{
		//Update this so the surface changes to other players
		APlayerCameraManager* CameraManager =  Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	
		if (CameraManager != NULL)
		{
			if (Target != NULL)
			{
				FVector Position = ConvertLocation(CameraManager->GetCameraLocation(), this, Target);
				FRotator Rotation = ConvertRotation(CameraManager->GetCameraRotation(), this, Target);
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



FVector APortal::ConvertLocation(FVector vLocation, AActor* aPortal, AActor* aTarget)
{
	FVector vPlayerDirection = vLocation - aPortal->GetActorLocation();
	FVector vDesiredLocation = aTarget->GetActorLocation();

	//Check if the PlayerDirection is Facing the Any Portal Vector
	//if = ~1 is facing that location
	FVector vDotProd;

	vDotProd.X = FVector::DotProduct(vPlayerDirection, aPortal->GetActorForwardVector());
	vDotProd.Y = FVector::DotProduct(vPlayerDirection, aPortal->GetActorRightVector());
	vDotProd.Z = FVector::DotProduct(vPlayerDirection, aPortal->GetActorUpVector());

	//Adjust the player's facing direction
	FVector vPlayerNewDirection = vDotProd.X * -aTarget->GetActorForwardVector() 
								+ vDotProd.Y * -aTarget->GetActorRightVector() 
								+ vDotProd.Z * aTarget->GetActorUpVector();

	FVector vConvertedRotation = vDesiredLocation + vPlayerNewDirection;

	return vConvertedRotation;
}


FRotator APortal::ConvertRotation(FRotator rRotation, AActor* aPortal, AActor* aTarget)
{
	FVector vWorldRotationAdjustment(0.f, 0.f, -180.f);
	FVector vLocalRotationAdjustment = FVector::ZeroVector;

	//Check if the float is almost equal
	//if so, validate the condition
	if (FVector::DotProduct(aPortal->GetActorForwardVector(), FVector::UpVector) > KINDA_SMALL_NUMBER)
	{
		//Make sure that the angle is beetween 180� & -180�
		vLocalRotationAdjustment.X = FMath::UnwindDegrees(aPortal->GetTransform().GetRotation().Euler().X);
		vLocalRotationAdjustment.Y = 180.f;
		vWorldRotationAdjustment.Z += vLocalRotationAdjustment.X;
	}
	else if (FVector::DotProduct(aPortal->GetActorForwardVector(), FVector::UpVector) < -KINDA_SMALL_NUMBER)
	{
		//Make sure that the angle is beetween 180� & -180�
		vLocalRotationAdjustment.X = FMath::UnwindDegrees(aPortal->GetTransform().GetRotation().Euler().X);
		vLocalRotationAdjustment.Y = -180.f;
		vWorldRotationAdjustment.Z -= vLocalRotationAdjustment.X;
	}

	//This quaternion represents an rotation arround an axis, since we want to rotate our character arround the z axis
	FQuat qRotationQuaternion = FQuat::MakeFromEuler(vWorldRotationAdjustment) * FQuat(rRotation);

	//Convert the rotation from world to local
	FQuat qLocalQuaternion = FQuat::MakeFromEuler(vLocalRotationAdjustment) * aPortal->GetActorTransform().GetRotation().Inverse() * qRotationQuaternion;

	FQuat qWorldQuaternion = aTarget->GetActorTransform().GetRotation() * qLocalQuaternion;

	FRotator rConvertedRotation = qWorldQuaternion.Rotator();

	return rConvertedRotation;
}



bool APortal::CheckIfIsInFrontOfPortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal)
{
	bool bIsInFrontOfPortal;

	//Create an Plane depending of the player direction
	FPlane PortalPlane = FPlane(vPortalLocation, vPortalNormal);

	//Validate the condition based on the distance 
	if (PortalPlane.PlaneDot(vPoint) >= 0)
	{
		bIsInFrontOfPortal = true;
	}
	else if (PortalPlane.PlaneDot(vPoint) < 0)
	{
		bIsInFrontOfPortal = false;
	}

	return bIsInFrontOfPortal;
}



bool APortal::CheckIfThePlayerIsCrossingThePortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal, bool LastInFront, FVector vLastPosition)
{

	FVector vMeetingPoint;
	FPlane pPortalSurface = FPlane(vPortalLocation, vPortalNormal);

	//Validates if LastPosition and Point meet their positions with the Plane
	bool bIsMeetingPositions = FMath::SegmentPlaneIntersection(vLastPosition, vPoint, pPortalSurface, vMeetingPoint);

	//Check if the player is crossing the portal with a forward or backwards movement
	bool bIsInFront = CheckIfIsInFrontOfPortal(vPoint, vPortalLocation, vPortalNormal);

	bool bIsCrossing = false;

	bLastInFront = CheckIfIsInFrontOfPortal(vPoint, vPortalLocation, vPortalNormal);

	vLastPosition = vPoint;

	if (bIsMeetingPositions && !bIsInFront && !bLastInFront)  bIsCrossing = true;


	return bIsCrossing;
}



