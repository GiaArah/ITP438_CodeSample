// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MultiPlayerState.generated.h"

UENUM(BlueprintType)
enum class EMultiTeam : uint8
{
	/* Team not assigned yet*/
	None,
	/* On red team */
	Red,
	/* On blue team */
	Blue
};

/**
 *
 */
UCLASS()
class MULTI_API AMultiPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMultiPlayerState();

	UPROPERTY(Replicated)
	EMultiTeam Team = EMultiTeam::None;

	UPROPERTY(Replicated)
	int KillStreak = 0;

	UFUNCTION(Server, Reliable)
	void SetKillStreak(int NewKillStreak);

	int GetKillStreak() const { return KillStreak; }

	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message);

	UFUNCTION(Server, Reliable)
	void SendReadyToStart(bool bIsReady);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
