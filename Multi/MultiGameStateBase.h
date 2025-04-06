// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MultiGameStateBase.generated.h"

/**
 *
 */
UCLASS()
class MULTI_API AMultiGameStateBase : public AGameState
{
	GENERATED_BODY()

public:
	AMultiGameStateBase();
	virtual void Tick(float DeltaSeconds) override;

	virtual void HandleMatchIsWaitingToStart() override;

	UPROPERTY(Replicated)
	int RedTeamScore = 0;

	UPROPERTY(Replicated)
	int BlueTeamScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float WaitingToStartTime;

	UFUNCTION(Server, Reliable)
	void SetNumPlayersReady(int NumReady);

	int GetNumPlayersReady() { return NumPlayersReady; }

	UPROPERTY(Replicated)
	int NumPlayersReady = 0;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTextAlert(const FString& Text, FLinearColor TextColor, float TextDuration);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
