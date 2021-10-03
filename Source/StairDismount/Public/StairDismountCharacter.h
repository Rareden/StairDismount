// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StairDismountCharacter.generated.h"

class UPrimitiveComponent;

UCLASS(config=Game)
class AStairDismountCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UArrowComponent* ForceDirectionArrow;

public:
	AStairDismountCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION(BlueprintCallable)
	void RagdollCharacter( const float ImpulsePower);

	UFUNCTION(BlueprintCallable)
	void ResetCharacater();

	// Set the velocity threshold and its Squared partner
	UFUNCTION(BlueprintSetter)
	void BP_SetLinearVelocityThreshold(const float Value);
	// Set the velocity threshold and its Squared partner
	UFUNCTION(BlueprintSetter)
	void BP_SetAngularVelocityThreshold(const float Value);

	void SetForceDirectionArrow(const FVector Dir, const FVector Location, const FName BoneName);

protected:
	void BeginPlay() override;

	void PossessedBy(AController* NewController)override;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnBoneTakeDamage(const FName BoneName, const int32 Amount);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnCharacterReset();

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	UPROPERTY()
		USkeletalMeshComponent* MeshPrivate;
	UPROPERTY()
		FTransform DefaultMeshTransform;
	// Squared thesholds for speed when comparing physics velocitys

	float LinearVelocityDmgThresholdSquared;
	float AngularVelocityDmgThresholdSquared;
	/** Bone to apply force to, set from player controller trace */
	FName TargetBone;

	//

	// Get world space forward direction that the arrow points in. 
	FORCEINLINE FVector GetArrowForceDirection()const;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	/** Linear Velocity size at which damage should be considered for the bone */
	UPROPERTY(EditAnywhere,  BlueprintSetter = BP_SetLinearVelocityThreshold)
		float LinearVelocityThreshold;

	/**Angular Velocity size at which damage should be considered for the bone */
	UPROPERTY(EditAnywhere, BlueprintSetter = BP_SetAngularVelocityThreshold)
		float AngularVelocityThreshold;

	UPROPERTY(BlueprintReadWrite)
	int32 TotalDamageTaken;

	// Multiplyer for damage 
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float DamageScalar;

	UPROPERTY(BlueprintReadOnly)
		bool bIsRagdolling;


public:
		
	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool GetIsRagdolling() const; 


	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};

