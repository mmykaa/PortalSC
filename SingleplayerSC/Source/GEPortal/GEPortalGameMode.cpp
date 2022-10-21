// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalGameMode.h"
#include "GEPortalHUD.h"
#include "GEPortalCharacter.h"
#include "Portal.h"

#include "DrawDebugHelpers.h"

#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AGEPortalGameMode::AGEPortalGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	//DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGEPortalHUD::StaticClass();

	fPortalHeight = 125.0f;
	fAdjustmentSubstep = 0.2f;
	maxAdjustbleSteps = fPortalHeight;
}


AGEPortalCharacter* AGEPortalGameMode::GetExistingPlayer()
{
	return aPlayer;
}

void AGEPortalGameMode::SetPlayer(AGEPortalCharacter* aExistingPlayer)
{
	aPlayer = aExistingPlayer;
}


void AGEPortalGameMode::SpawnBluePortal(const FHitResult& Hit, AActor* Projectile)
{
	Projectile->Destroy();

	UWorld* const World = GetWorld();

	if (World != nullptr)
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector Origin = Hit.Location + Hit.ImpactNormal;

		FRotator Rotation = Hit.ImpactNormal.Rotation();

		//Limit the number of portals to one of each
		if (aBluePortalSpawned != NULL)
		{
			aBluePortalSpawned->Destroy();
		}

		aBluePortalSpawned = World->SpawnActor<APortal>(aBluePortalToSpawn, Origin, Rotation, ActorSpawnParams);

		//Update both portals targets
		AGEPortalGameMode::UpdatePortalTargets();

		AGEPortalGameMode::CheckPortalPlacement(aBluePortalSpawned, Origin);
	}
}


void AGEPortalGameMode::SpawnOrangePortal(const FHitResult& Hit, AActor* Projectile)
{

	Projectile->Destroy();

	UWorld* const World = GetWorld();

	if (World != nullptr)
	{

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector Origin = Hit.Location + Hit.ImpactNormal;
		FRotator Rotation = Hit.ImpactNormal.Rotation();


		//Limit the number of portals to one of each
		if (aOrangePortalSpawned != NULL)
		{
			aOrangePortalSpawned->Destroy();
		}

		aOrangePortalSpawned = World->SpawnActor<APortal>(aOrangePortalToSpawn, Origin, Rotation, ActorSpawnParams);

		UE_LOG(LogTemp, Warning, TEXT("Orange SPAWNED"));

		//Update both portals targets
		AGEPortalGameMode::UpdatePortalTargets();

		AGEPortalGameMode::CheckPortalPlacement(aOrangePortalSpawned, Origin);
	}
}


void AGEPortalGameMode::UpdatePortalTargets()
{
	if (aOrangePortalSpawned == NULL || aBluePortalSpawned == NULL)
	{
		return;
	}
	else
	{
		Cast<APortal>(aBluePortalSpawned)->SetTarget(aOrangePortalSpawned);
		Cast<APortal>(aOrangePortalSpawned)->SetTarget(aBluePortalSpawned);
	}
}



void AGEPortalGameMode::CheckPortalPlacement(APortal* PortalToCheckLocation, FVector ProjectileImpactLocation)
{
	//Save first hit position
	FVector ProjectileImpactLocationBefore = ProjectileImpactLocation;

	//Get Portal Fwrd Right Up Vectors
	FVector ForwardVector = PortalToCheckLocation->GetActorForwardVector();
	FVector RightVector = PortalToCheckLocation->GetActorRightVector();
	FVector UpVector = PortalToCheckLocation->GetActorUpVector();

	//Check if is overlapping another portal
	APortal* aThisPortalTravellingTarget = PortalToCheckLocation->GetTarget();

	//Check if fits in the wall
	//Get the Extent Box Created by the portal
	FVector PortalBoxCollider = PortalToCheckLocation->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();

	//Get the Box Collider edges
	FVector UpperEdge = UpVector * PortalBoxCollider.Z;
	FVector RightEdge = RightVector * PortalBoxCollider.Y;
	FVector BottomEdge = UpVector * -PortalBoxCollider.Z;
	FVector LeftEdge = RightVector * -PortalBoxCollider.Y;

	AGEPortalGameMode::FixPortalPlacementInWall(PortalToCheckLocation, UpperEdge, RightEdge, BottomEdge, LeftEdge, ProjectileImpactLocation);

}

void AGEPortalGameMode::FixPortalPlacementInWall(APortal* PortalToFixLocation, FVector Upper, FVector Right, FVector Bottom, FVector Left, FVector ProjectileImpactLocation)
{
	//Set initialarray Size
	PortalBoxCorners.SetNumUninitialized(8);

	//Set the Corners and the center of the portal
	PortalBoxCenter = ProjectileImpactLocation;

	PortalBoxCorners[0] = ProjectileImpactLocation + Upper + Left;     //     0______1______2
	PortalBoxCorners[1] = ProjectileImpactLocation + Upper;            //	  |             |
	PortalBoxCorners[2] = ProjectileImpactLocation + Upper + Right;    //	  |             |
	PortalBoxCorners[3] = ProjectileImpactLocation + Right;            //	  |             |
	PortalBoxCorners[4] = ProjectileImpactLocation + Bottom + Right;   //	  7      x      3
	PortalBoxCorners[5] = ProjectileImpactLocation + Bottom;           //     |             |
	PortalBoxCorners[6] = ProjectileImpactLocation + Bottom + Left;    //	  |             |
	PortalBoxCorners[7] = ProjectileImpactLocation + Left;             //     6______5______4


	FHitResult HitResult;

	//Check From Center the wall that the portal landed
	LineTraceFromCenter(ProjectileImpactLocation, PortalToFixLocation);

	//If all the following checks return null the portal must be deleted since is not in a wall
	//Check if the once of these traces is null if so, the portal is not properly landed
	//Then Adjust his location
	if (WallFound != NULL)
	{
		LineTraceFromCorner(PortalBoxCorners[0], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[1], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[2], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[3], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[4], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[5], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[6], PortalToFixLocation, ProjectileImpactLocation);
		LineTraceFromCorner(PortalBoxCorners[7], PortalToFixLocation, ProjectileImpactLocation);
	}
}


void AGEPortalGameMode::LineTraceFromCenter(FVector Center, APortal* PortalToFixLocation)
{
	FVector ForwardVector = PortalToFixLocation->GetActorForwardVector();
	FVector TraceExtend = Center - ForwardVector * 40.0f;

	FHitResult Hit;
	FCollisionQueryParams LineTraceParams;

	LineTraceParams.bTraceComplex = true;
	LineTraceParams.AddIgnoredActor(PortalToFixLocation);
	LineTraceParams.AddIgnoredActor(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	LineTraceParams.AddIgnoredActor(GetExistingPlayer());

	GetWorld()->LineTraceSingleByChannel(Hit, Center, TraceExtend, ECC_Visibility, LineTraceParams);

	//DrawDebugLine(GetWorld(), Center, TraceExtend, FColor::Red, false, 3.0f);

	if (Hit.GetActor() != NULL)
	{
		WallFound = Hit.GetActor();
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName()));
	}
	else
	{
		//If Somehow the portals spawns in a null center destroy him
		PortalToFixLocation->Destroy();
	}
}


void AGEPortalGameMode::LineTraceFromCorner(FVector Corner, APortal* PortalToFixLocation, FVector ProjectileImpactLocation)
{
	FVector ForwardVector = PortalToFixLocation->GetActorForwardVector();
	FVector RightVector = PortalToFixLocation->GetActorRightVector();
	FVector UpVector = PortalToFixLocation->GetActorUpVector();

	FVector TraceStart = ProjectileImpactLocation - ForwardVector;
	FVector TraceEnd = Corner - ForwardVector;
	FVector TraceExtend = TraceEnd - ForwardVector * 40.0f;

	FHitResult Hit;
	FCollisionQueryParams LineTraceParams;

	LineTraceParams.bTraceComplex = true;
	LineTraceParams.AddIgnoredActor(PortalToFixLocation);
	LineTraceParams.AddIgnoredActor(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	LineTraceParams.AddIgnoredActor(GetExistingPlayer());

	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, LineTraceParams);
	GetWorld()->LineTraceSingleByChannel(Hit, TraceEnd, TraceExtend, ECC_Visibility, LineTraceParams);

	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false, 3.0f);
	//DrawDebugLine(GetWorld(), TraceEnd, TraceExtend, FColor::Red, false, 3.0f);

	if (Hit.GetActor() == NULL)
	{
		AdjustLocation(PortalToFixLocation, Corner, ProjectileImpactLocation, Hit);
	}
	else
	{
		//	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName()));
	}
}


void AGEPortalGameMode::AdjustLocation(APortal* PortalToFixLocation, FVector CornerToAdjust, FVector ProjectileImpactLocation, FHitResult Hit)
{
	//Get Portal Right and Up Vectors
	FVector RightVector = PortalToFixLocation->GetActorRightVector();
	FVector UpVector = PortalToFixLocation->GetActorUpVector();

	//Get these to repeat the trace
	FVector ForwardVector = PortalToFixLocation->GetActorForwardVector();
	FVector TraceStart = ProjectileImpactLocation - ForwardVector;
	FVector TraceEnd = CornerToAdjust - ForwardVector;
	FVector TraceExtend = TraceEnd - ForwardVector * 40.0f;


	//RepeatTraceParams
	FHitResult ReapeatedHit;
	FCollisionQueryParams LineTraceParams;

	LineTraceParams.bTraceComplex = true;
	LineTraceParams.AddIgnoredActor(PortalToFixLocation);
	LineTraceParams.AddIgnoredActor(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	LineTraceParams.AddIgnoredActor(GetExistingPlayer());


	//Check if is a vertical point or horizontal,
	if (CornerToAdjust == PortalBoxCorners[1] || CornerToAdjust == PortalBoxCorners[3] || CornerToAdjust == PortalBoxCorners[5] || CornerToAdjust == PortalBoxCorners[7])
	{
		//Upper Vertical Point
		if (CornerToAdjust == PortalBoxCorners[1])
		{
			UE_LOG(LogTemp, Warning, TEXT("Upper Point is Out"));

			if (Hit.GetActor() == NULL)
			{

				bool finishedLooping = false;

				for (float i = fAdjustmentSubstep; i < maxAdjustbleSteps; ++i)
				{
					if (!finishedLooping)
					{

						UE_LOG(LogTemp, Warning, TEXT("Upper Point is STILL Out"));

						//Update Trace Positions
						PortalToFixLocation->SetActorLocation(PortalToFixLocation->GetActorLocation() + -UpVector * (i / i));

						TraceStart = PortalToFixLocation->GetActorLocation() - ForwardVector;
						
						FVector PortalBoxCollider = PortalToFixLocation->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();

						FVector UpperEdge = UpVector * PortalBoxCollider.Z;
						
						CornerToAdjust = PortalToFixLocation->GetActorLocation() + UpperEdge;

						TraceEnd = CornerToAdjust - ForwardVector;
						TraceExtend = TraceEnd - ForwardVector * 40.0f;

						GetWorld()->LineTraceSingleByChannel(ReapeatedHit, TraceEnd, TraceExtend, ECC_Visibility, LineTraceParams);

						//	DrawDebugLine(GetWorld(), TraceEnd, TraceExtend, FColor::Purple, false, 3.0f);

						if (i == maxAdjustbleSteps && ReapeatedHit.GetActor() == NULL)
						{
							PortalToFixLocation->Destroy();
						}
		
						if (ReapeatedHit.GetActor() != NULL)
						{
							UE_LOG(LogTemp, Warning, TEXT("Upper Point is IN IN IN"));
							finishedLooping = true;
						}
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Upper Is In the Final Position"));
			}
		}

		//Right Horizontal Point
		if (CornerToAdjust == PortalBoxCorners[3])
		{
			UE_LOG(LogTemp, Warning, TEXT("Right Point is Out"));

			if (Hit.GetActor() == NULL)
			{

				bool finishedLooping = false;

				for (float i = fAdjustmentSubstep; i < maxAdjustbleSteps; ++i)
				{
					if (!finishedLooping)
					{
						UE_LOG(LogTemp, Warning, TEXT("Upper Point is STILL Out"));

						//Update Trace Positions
						PortalToFixLocation->SetActorLocation(PortalToFixLocation->GetActorLocation() + -RightVector * (i / i));

						TraceStart = PortalToFixLocation->GetActorLocation() - ForwardVector;

						FVector PortalBoxCollider = PortalToFixLocation->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();
												
						FVector RightEdge = RightVector * PortalBoxCollider.Y;
						
						CornerToAdjust = PortalToFixLocation->GetActorLocation() + RightEdge;

						TraceEnd = CornerToAdjust - ForwardVector;
						TraceExtend = TraceEnd - ForwardVector * 40.0f;

						GetWorld()->LineTraceSingleByChannel(ReapeatedHit, TraceEnd, TraceExtend, ECC_Visibility, LineTraceParams);

						//	DrawDebugLine(GetWorld(), TraceEnd, TraceExtend, FColor::Purple, false, 3.0f);

						if (i == maxAdjustbleSteps && ReapeatedHit.GetActor() == NULL)
						{
							PortalToFixLocation->Destroy();
						}

						if (ReapeatedHit.GetActor() != NULL)
						{
							UE_LOG(LogTemp, Warning, TEXT("Upper Point is IN IN IN"));
							finishedLooping = true;
						}
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Right Is In the Final Position"));
			}
		}

		//Bottom Vertical Point
		if (CornerToAdjust == PortalBoxCorners[5])
		{
			UE_LOG(LogTemp, Warning, TEXT("Bottom Point is Out"));

			if (Hit.GetActor() == NULL)
			{
				
				bool finishedLooping = false;

				for (float i = fAdjustmentSubstep; i < maxAdjustbleSteps; ++i)
				{
					if (!finishedLooping)
					{

						UE_LOG(LogTemp, Warning, TEXT("Bottom Point is STILL Out"));

						//Update Trace Positions
						PortalToFixLocation->SetActorLocation(PortalToFixLocation->GetActorLocation() + UpVector * (i / i));

						TraceStart = PortalToFixLocation->GetActorLocation() - ForwardVector;

						FVector PortalBoxCollider = PortalToFixLocation->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();

						FVector BottomEdge = UpVector * -PortalBoxCollider.Z;

						CornerToAdjust = PortalToFixLocation->GetActorLocation() + BottomEdge;

						TraceEnd = CornerToAdjust - ForwardVector;
						TraceExtend = TraceEnd - ForwardVector * 40.0f;

						GetWorld()->LineTraceSingleByChannel(ReapeatedHit, TraceEnd, TraceExtend, ECC_Visibility, LineTraceParams);

						//	DrawDebugLine(GetWorld(), TraceEnd, TraceExtend, FColor::Purple, false, 3.0f);

						if (i == maxAdjustbleSteps && ReapeatedHit.GetActor() == NULL)
						{
							PortalToFixLocation->Destroy();
						}

						if (ReapeatedHit.GetActor() != NULL)
						{
							UE_LOG(LogTemp, Warning, TEXT("Bottom Point is IN"));
							finishedLooping = true;
						}
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Bottom Is In the Final Position"));
			}
		}

		//Left Horizontal Point
		if (CornerToAdjust == PortalBoxCorners[7])
		{
			UE_LOG(LogTemp, Warning, TEXT("Left Point is Out"));

			if (Hit.GetActor() == NULL)
			{
				bool finishedLooping = false;

				for (float i = fAdjustmentSubstep; i < maxAdjustbleSteps; ++i)
				{
					if (!finishedLooping)
					{

						UE_LOG(LogTemp, Warning, TEXT("Left Point is STILL Out"));

						//Update Trace Positions
						PortalToFixLocation->SetActorLocation(PortalToFixLocation->GetActorLocation() + RightVector* (i / i));

						TraceStart = PortalToFixLocation->GetActorLocation() - ForwardVector;

						FVector PortalBoxCollider = PortalToFixLocation->CalculateComponentsBoundingBoxInLocalSpace().GetExtent();

						FVector LeftEdge = RightVector * -PortalBoxCollider.Y;

						CornerToAdjust = PortalToFixLocation->GetActorLocation() + LeftEdge;

						TraceEnd = CornerToAdjust - ForwardVector;
						TraceExtend = TraceEnd - ForwardVector * 40.0f;

						GetWorld()->LineTraceSingleByChannel(ReapeatedHit, TraceEnd, TraceExtend, ECC_Visibility, LineTraceParams);

						//	DrawDebugLine(GetWorld(), TraceEnd, TraceExtend, FColor::Purple, false, 3.0f);

						if (i == maxAdjustbleSteps && ReapeatedHit.GetActor() == NULL)
						{
							PortalToFixLocation->Destroy();
						}

						if (ReapeatedHit.GetActor() != NULL)
						{
							UE_LOG(LogTemp, Warning, TEXT("Left Point is IN"));
							finishedLooping = true;
						}
					}
				}

				UE_LOG(LogTemp, Warning, TEXT("Left Is In the Final Position"));
			}
		}
	}




	//if it is the upper vertical point
	//add a fraction of the up vector

	//if it is the bottom vertical point
	//subtract a fraction of the up vector

	//if it is the left horizontal point
	//add a fraction of the up vector

	//if it is the right horizontal point
	//add a fraction of the up vector

	//Check if already found the wall,
	//if so stop
	//if not repeat
}



FVector AGEPortalGameMode::ConvertLocation(FVector vLocation, AActor* aPortal, AActor* aTarget)
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


FRotator AGEPortalGameMode::ConvertRotation(FRotator rRotation, AActor* aPortal, AActor* aTarget)
{
	FVector vWorldRotationAdjustment(0.f, 0.f, -180.f);
	FVector vLocalRotationAdjustment = FVector::ZeroVector;

	//Check if the float is almost equal
	//if so, validate the condition
	if (FVector::DotProduct(aPortal->GetActorForwardVector(), FVector::UpVector) > KINDA_SMALL_NUMBER)
	{
		//Make sure that the angle is beetween 180º & -180º
		vLocalRotationAdjustment.X = FMath::UnwindDegrees(aPortal->GetTransform().GetRotation().Euler().X);
		vLocalRotationAdjustment.Y = 180.f;
		vWorldRotationAdjustment.Z += vLocalRotationAdjustment.X;
	}
	else if (FVector::DotProduct(aPortal->GetActorForwardVector(), FVector::UpVector) < -KINDA_SMALL_NUMBER)
	{
		//Make sure that the angle is beetween 180º & -180º
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



bool AGEPortalGameMode::CheckIfIsInFrontOfPortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal)
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



bool AGEPortalGameMode::CheckIfThePlayerIsCrossingThePortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal, bool bLastInFront, FVector vLastPosition)
{

	FVector vMeetingPoint;
	FPlane PortalSurface = FPlane(vPortalLocation, vPortalNormal);

	//Validates if LastPosition and Point meet their positions with the Plane
	bool bIsMeetingPositions = FMath::SegmentPlaneIntersection(vLastPosition, vPoint, PortalSurface, vMeetingPoint);

	//Check if the player is crossing the portal with a forward or backwards movement
	bool bIsInFront = CheckIfIsInFrontOfPortal(vPoint, vPortalLocation, vPortalNormal);

	bool bIsCrossing = false;

	bLastInFront = CheckIfIsInFrontOfPortal(vPoint, vPortalLocation, vPortalNormal);

	vLastPosition = vPoint;

	if (bIsMeetingPositions && !bIsInFront && !bLastInFront)  bIsCrossing = true;


	return bIsCrossing;
}

