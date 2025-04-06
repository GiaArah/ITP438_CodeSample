// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MultiSaveGame.generated.h"

USTRUCT()
struct FTagActorSaveData
{
	GENERATED_BODY()

public:
	// Save transform (position, rotation, scale) of tag
	UPROPERTY()
	FTransform Transform;

	// Type of TagActor this is
	UPROPERTY()
	TSubclassOf<class ATagActor> ClassType;
};

/**
 *
 */
UCLASS()
class MULTI_API UMultiSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	void AddTag(FTagActorSaveData NewTag) { TagActors.Add(NewTag); }

	TArray<FTagActorSaveData> GetTagActors() { return TagActors; }

protected:
	UPROPERTY()
	TArray<FTagActorSaveData> TagActors;
};
