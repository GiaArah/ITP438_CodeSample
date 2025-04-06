// Copywright 2023 Gia Lang

#include "MultiPlayerState.h"
#include "MultiGameStateBase.h"
#include "MultiPlayerController.h"
#include "HUDWidget.h"
#include "Net/UnrealNetwork.h"

void AMultiPlayerState::ServerSendChatMessage_Implementation(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message)
{
	AMultiGameStateBase*  GameState = Cast<AMultiGameStateBase>(GetWorld()->GetGameState());
	TArray<APlayerState*> PlayerStates = GameState->PlayerArray;

	for (int i = 0; i < PlayerStates.Num(); i++)
	{
		if (AMultiPlayerState* PS = Cast<AMultiPlayerState>(PlayerStates[i]))
		{
			if (ChatTeam == PS->Team || ChatTeam == EMultiTeam::None)
			{
				PS->ClientReceiveChatMessage(ChatTeam, SenderName, Message);
			}
		}
	}
}

void AMultiPlayerState::ClientReceiveChatMessage_Implementation(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message)
{
	AMultiPlayerController* PlayerController = GetOwner<AMultiPlayerController>();
	if (PlayerController)
	{
		PlayerController->GetHUDWidgetInstance()->AddChatMessage(ChatTeam, SenderName, Message);
	}
}

void AMultiPlayerState::SendReadyToStart_Implementation(bool bIsReady)
{
	AMultiGameStateBase* GameState = Cast<AMultiGameStateBase>(GetWorld()->GetGameState());
	if (GameState)
	{
		if (bIsReady)
		{
			GameState->SetNumPlayersReady(GameState->GetNumPlayersReady() + 1);
		}
		else
		{
			GameState->SetNumPlayersReady(GameState->GetNumPlayersReady() - 1);
		}
	}
}

void AMultiPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiPlayerState, Team);
	DOREPLIFETIME(AMultiPlayerState, KillStreak);
}

AMultiPlayerState::AMultiPlayerState()
{
	NetUpdateFrequency = 4.0f;
}

void AMultiPlayerState::SetKillStreak_Implementation(int NewKillStreak)
{
	KillStreak = NewKillStreak;
}
