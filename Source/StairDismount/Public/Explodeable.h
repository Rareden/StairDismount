// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Explodeable.generated.h"

UCLASS()
class STAIRDISMOUNT_API AExplodeable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplodeable();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USphereComponent* TriggerSphere;
	/** Max force applied to overlapping object */
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		float ExplosiveForce;
	/** BP exposed event after object explodes and applys force, do visual FX from here */
	UFUNCTION(BlueprintImplementableEvent)
	void OnExplode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** On trigger sphere overlap */
	UFUNCTION()
		void OnTriggerHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/** Explode the object apply force to hit component */
	void Explode(UPrimitiveComponent* OtherComp);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
