// Copyright Epic Games, Inc. All Rights Reserved.

#include "NNEexampleGameMode.h"
#include "NNEexampleCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANNEexampleGameMode::ANNEexampleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
