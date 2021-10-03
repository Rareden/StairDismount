// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StairDismountController.generated.h"

class AStairDismountCharacter;

/**
 * 
 */
UCLASS()
class STAIRDISMOUNT_API AStairDismountController : public APlayerController
{
	GENERATED_BODY()
	

	void SetupInputComponent() override;
	void PlayerTick(float DeltaTime) override;
	
	void TraceMouse();
public:
	/** get last mouse trace result */
	FHitResult GetLastCharacterHit() const
	{
		return LastCharacterHit;
	}
	/** Left mouse button click*/
	void OnLMB();
protected:
	void OnPossess(APawn* aPawn) override;

private:
	FHitResult LastCharacterHit;
	/** Soft ptr to player character */
	TWeakObjectPtr<AStairDismountCharacter> PrivateCharacter;

};
