// Fill out your copyright notice in the Description page of Project Settings.


#include "StairDismountController.h"
#include "StairDismountCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

void AStairDismountController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if(InputComponent)
	{
		InputComponent->BindAction("LMB",IE_Released,this,&AStairDismountController::OnLMB);
	}
}

void AStairDismountController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	TraceMouse();
}

void AStairDismountController::TraceMouse()
{
	if(!GetWorld() || GetWorld()->bIsTearingDown || !PlayerCameraManager) return;
	

	FVector2D MousePos;
	if(GetMousePosition(MousePos.X,MousePos.Y))
	{
		FVector WorldPos, WorldDir;
		UGameplayStatics::DeprojectScreenToWorld(this, MousePos, WorldPos, WorldDir);


		TArray<AActor*> Ignores;
		FHitResult Hit; 

		GetWorld()->LineTraceSingleByObjectType(Hit,WorldPos, WorldPos + (WorldDir * 1400.f),ECC_WorldDynamic);

	
		if (Hit.bBlockingHit && Hit.Actor.IsValid())
		{
			if(AStairDismountCharacter* Char = Cast<AStairDismountCharacter>(Hit.Actor.Get()))
			{
				DrawDebugPoint(GetWorld(),Hit.Location,25.f,FColor::Red);

				
				GEngine->AddOnScreenDebugMessage(-1,-1.f,FColor::Red,Hit.BoneName.ToString());
			}
		}
		LastCharacterHit = Hit;
	}
}

void AStairDismountController::OnLMB()
{
	if(LastCharacterHit.bBlockingHit && PrivateCharacter.IsValid())
	{
		
		PrivateCharacter.Get()->SetForceDirectionArrow(PlayerCameraManager->GetTransform().GetRotation().GetForwardVector(),LastCharacterHit.Location,LastCharacterHit.BoneName);

	}
}

void AStairDismountController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	if(AStairDismountCharacter* Char = Cast<AStairDismountCharacter>(aPawn))
	{
		PrivateCharacter = Char;
	}
}
