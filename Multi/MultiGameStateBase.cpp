// Copywright 2023 Gia Lang

#include "MultiGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "MultiGameMode.h"
#include "MultiPlayerController.h"

AMultiGameStateBase::AMultiGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMultiGameStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (GetMatchState() == MatchState::WaitingToStart && (PlayerArray.Num() > 0) && (PlayerArray.Num() == NumPlayersReady))
	{
		WaitingToStartTime -= DeltaSeconds;
		if (WaitingToStartTime <= 0.0f)
		{
			WaitingToStartTime = 0.0f;
			if (GetLocalRole() == ROLE_Authority)
			{
				if (AMultiGameMode* GameMode = GetWorld()->GetAuthGameMode<AMultiGameMode>())
				{
					GameMode->StartMatch();
					MulticastTextAlert(FString(TEXT("STARTING MATCH")), FLinearColor::Green, 3.0f);
				}
			}
		}
	}
}

void AMultiGameStateBase::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	if (GetLocalRole() == ROLE_Authority)
	{
		WaitingToStartTime = GetDefaultGameMode<AMultiGameMode>()->WaitingToStartDuration;
	}
}

void AMultiGameStateBase::MulticastTextAlert_Implementation(const FString& Text, FLinearColor TextColor, float TextDuration)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController*		PC = It->Get();
		AMultiPlayerController* MultiPC = Cast<AMultiPlayerController>(PC);

		if (MultiPC)
		{
			if (MultiPC->IsLocalController())
			{
				// add player controller and HUD widget functions
				MultiPC->SendTextAlertData(Text, TextColor, TextDuration);
			}
		}
	}
}

void AMultiGameStateBase::SetNumPlayersReady_Implementation(int NumReady)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		NumPlayersReady = NumReady;
	}
}

void AMultiGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiGameStateBase, RedTeamScore);
	DOREPLIFETIME(AMultiGameStateBase, BlueTeamScore);
	DOREPLIFETIME(AMultiGameStateBase, NumPlayersReady);

	// conditionally replicate the waiting to start time on initial replication
	DOREPLIFETIME_CONDITION(AMultiGameStateBase, WaitingToStartTime, COND_InitialOnly);
}

