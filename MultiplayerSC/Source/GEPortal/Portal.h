// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEPortalCharacter.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

class UMaterial;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

UCLASS()
class GEPORTAL_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

	virtual void Tick(float DeltaTime) override;


	AGEPortalCharacter* PlayerWhoFiredProjectile;
	
	UFUNCTION() void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION() void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION() void Teleport();

	UFUNCTION() void CheckIfCanTeleport();

	UFUNCTION() void RedirectShot(FHitResult OutHit, FRotator OnShootPlayerRotation, AActor* Shooter);

	UFUNCTION() void RedirectProjectile(AActor* Projectile, FVector Location, FRotator OnShootPlayerRotation, AGEPortalCharacter* Shooter);

	UFUNCTION() void SetTarget(APortal* NewTarget);

	UFUNCTION() void SetPortalSurface(AActor* Surface);
	
	AActor* GetPortalSurface() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> RedirectHelper;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TSubclassOf<AActor> RocketProjectile;
	APortal* GetTarget() const;

	bool bFoundActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capture", Meta = (AllowPrivateAccess = true))
	USceneCaptureComponent2D* SceneCaptureComponent;
	USceneComponent* BackfacedPortal;
	
	void UpdatePortalSurfaces();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		UStaticMeshComponent* PortalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		UStaticMeshComponent* BorderMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Portal)
		UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
		APortal* Target;

private:

	FVector LastPosition;

	bool bLastInFront;

	bool Overlapping;

	AActor* PortalSurface;

	TArray<AActor*> OverlappingActors;

	FTimerHandle fthCheckIfCanTeleport;
	
bool bIsTeleporting;

	UFUNCTION()
		 FVector ConvertLocation(FVector vlocation, AActor* aPortal, AActor* aTarget);

	UFUNCTION()
		 FRotator ConvertRotation(FRotator rRotation, AActor* aPortal, AActor* aTarget);

	UFUNCTION()
		 bool CheckIfIsInFrontOfPortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal);

	UFUNCTION()
		 bool CheckIfThePlayerIsCrossingThePortal(FVector vPoint, FVector vPortalLocation, FVector vPortalNormal, bool bLastInFront, FVector bLastPosition);


	AActor* ObjectToTeleport;

	AActor* ObjectOverlapping;
	

};

