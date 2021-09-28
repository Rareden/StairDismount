// Copyright Epic Games, Inc. All Rights Reserved.

#include "StairDismountGameMode.h"
#include "StairDismountCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStairDismountGameMode::AStairDismountGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
