// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiSaveSystem.generated.h"

/**
 *
 */
UCLASS()
class MULTI_API UMultiSaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void CreateNewSave();

	void SaveGame(bool bShouldAsyncSave);

	void LoadGame();

	bool SaveFilePresent();

	TObjectPtr<class UMultiSaveGame> GetSave() { return GroundTruthSave; }

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UPROPERTY()
	TObjectPtr<class UMultiSaveGame> GroundTruthSave;
};
