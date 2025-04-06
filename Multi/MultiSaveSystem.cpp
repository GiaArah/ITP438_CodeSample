#include "MultiSaveSystem.h"
// Copywright 2023 Gia Lang

#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "TagActor.h"

void UMultiSaveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GroundTruthSave = NewObject<UMultiSaveGame>();
}

void UMultiSaveSystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMultiSaveSystem::CreateNewSave()
{
	GroundTruthSave = NewObject<UMultiSaveGame>();
	SaveGame(false);
}

void UMultiSaveSystem::SaveGame(bool bShouldAsyncSave)
{
	if (bShouldAsyncSave)
	{
		UGameplayStatics::AsyncSaveGameToSlot(GroundTruthSave, FString(TEXT("SaveGame")), 0);
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(GroundTruthSave, FString(TEXT("SaveGame")), 0);
	}
}

void UMultiSaveSystem::LoadGame()
{
	UMultiSaveGame* LoadedSave = Cast<UMultiSaveGame>(UGameplayStatics::LoadGameFromSlot(FString(TEXT("SaveGame")), 0));
	if (LoadedSave)
	{
		// load succeeded, change save to current one & spawn actors
		GroundTruthSave = LoadedSave;
		for (int i = 0; i < GroundTruthSave->GetTagActors().Num(); i++)
		{

			ATagActor* TagActorInstance = GetWorld()->SpawnActor<ATagActor>(GroundTruthSave->GetTagActors()[i].ClassType, GroundTruthSave->GetTagActors()[i].Transform.GetLocation(), FRotator::ZeroRotator);
			if (TagActorInstance)
			{
				TagActorInstance->SetActorRotation(GroundTruthSave->GetTagActors()[i].Transform.GetRotation());
				TagActorInstance->SetActorScale3D(GroundTruthSave->GetTagActors()[i].Transform.GetScale3D());
			}
		}
	}
}

bool UMultiSaveSystem::SaveFilePresent()
{
	if (UGameplayStatics::DoesSaveGameExist(FString(TEXT("SaveGame")), 0))
	{
		return true;
	}
	else
	{
		return false;
	}
}
