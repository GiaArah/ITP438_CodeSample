// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MultiGameMode.generated.h"

UCLASS(minimalapi)
class AMultiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMultiGameMode();

	// amount of time in seconds before game starts
	UPROPERTY(EditDefaultsOnly)
	float WaitingToStartDuration = 5.0f;

protected:
	virtual void GenericPlayerInitialization(AController* C) override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual bool	ShouldSpawnAtStartSpot(AController* Player) override { return false; }

	virtual bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;

	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	int RedTeamNumPlayers = 0;
	int BlueTeamNumPlayers = 0;

	FTimerHandle MatchEndTimer;
	FTimerHandle AutoSaveRequestTimer;
	FTimerHandle AutoSaveThrobberTimer;

	void HideThrobber();

	void AutoSave();
	void Save();
};
