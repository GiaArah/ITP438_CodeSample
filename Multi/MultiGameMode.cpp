// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiGameMode.h"
#include "MultiCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "MultiPlayerState.h"
#include "MultiPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "MultiGameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "HUDWidget.h"

AMultiGameMode::AMultiGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerStateClass = AMultiPlayerState::StaticClass();
	GameStateClass = AMultiGameStateBase::StaticClass();

	bDelayedStart = true;
}

bool AMultiGameMode::ReadyToEndMatch_Implementation()
{
	AMultiGameStateBase* MultiGSB = GetGameState<AMultiGameStateBase>();
	if (MultiGSB)
	{
		if (MultiGSB->RedTeamScore >= 10)
		{
			return true;
		}
		else if (MultiGSB->BlueTeamScore >= 10)
		{
			return true;
		}
	}
	return false;
}

void AMultiGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	AMultiGameStateBase* MultiGSB = GetGameState<AMultiGameStateBase>();
	if (MultiGSB)
	{
		FString		 AlertString;
		FLinearColor AlertColor;
		if (MultiGSB->RedTeamScore > MultiGSB->BlueTeamScore)
		{
			AlertString = TEXT("RED TEAM WINS");
			AlertColor = FLinearColor::Red;
		}
		else
		{
			AlertString = TEXT("BLUE TEAM WINS");
			AlertColor = FLinearColor::Blue;
		}

		MultiGSB->MulticastTextAlert(AlertString, AlertColor, 5.0f);
	}

	Save();

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(MatchEndTimer, this, &AMultiGameMode::RestartGame, 5.0f);
}

void AMultiGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	UMultiSaveSystem* NewSave = GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (NewSave)
	{
		NewSave->LoadGame();
	}
}

void AMultiGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(AutoSaveRequestTimer, this, &AMultiGameMode::AutoSave, 5.0f, true);
}

void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Save();
	Super::EndPlay(EndPlayReason);
}

void AMultiGameMode::HideThrobber()
{
	AMultiPlayerController* MyPC = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (MyPC)
	{
		MyPC->GetHUDWidgetInstance()->ToggleSaveThrobber(false);
	}
}

void AMultiGameMode::AutoSave()
{
	AMultiPlayerController* MyPC = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (MyPC)
	{
		MyPC->GetHUDWidgetInstance()->ToggleSaveThrobber(true);
		FTimerManager& TimerManager = GetWorldTimerManager();
		TimerManager.SetTimer(AutoSaveThrobberTimer, this, &AMultiGameMode::HideThrobber, 0.25f);
	}

	UMultiSaveSystem* NewSave = GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (NewSave)
	{
		NewSave->SaveGame(true);
	}
}

void AMultiGameMode::Save()
{
	UMultiSaveSystem* NewSave = GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (NewSave)
	{
		NewSave->SaveGame(false);
	}
}

void AMultiGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	// check if player already has a team assigned to their player state
	AMultiPlayerController* MyPC = C ? Cast<AMultiPlayerController>(C) : nullptr;
	if (MyPC)
	{
		AMultiPlayerState* PlayerState = MyPC ? Cast<AMultiPlayerState>(MyPC->PlayerState) : nullptr;
		if (PlayerState && PlayerState->Team == EMultiTeam::None)
		{
			// assign a team based on number of team members
			if (BlueTeamNumPlayers < RedTeamNumPlayers)
			{
				PlayerState->Team = EMultiTeam::Blue;
				BlueTeamNumPlayers++;
			}
			else
			{
				PlayerState->Team = EMultiTeam::Red;
				RedTeamNumPlayers++;
			}
		}
	}
}

AActor* AMultiGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Choose a player start
	APlayerStart*		  FoundPlayerStart = nullptr;
	UClass*				  PawnClass = GetDefaultPawnClassForController(Player);
	APawn*				  PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;
	TArray<APlayerStart*> BackupStartPoints;
	UWorld*				  World = GetWorld();

	FName	   PlayerStartTag = NAME_None;
	EMultiTeam Team = Player->GetPlayerState<AMultiPlayerState>()->Team;
	if (Team == EMultiTeam::Red)
	{
		PlayerStartTag = "Red";
	}
	else if (Team == EMultiTeam::Blue)
	{
		PlayerStartTag = "Blue";
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		else if (PlayerStart->PlayerStartTag == FName(TEXT("Backup")))
		{
			BackupStartPoints.Add(PlayerStart);
		}
		else if (PlayerStartTag == NAME_None || PlayerStartTag == PlayerStart->PlayerStartTag)
		{
			FVector		   ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				UnOccupiedStartPoints.Add(PlayerStart);
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				OccupiedStartPoints.Add(PlayerStart);
			}
		}
	}
	// remove from unoccupiedstartpoints and occupiedstartpoints any playerstarts
	// which there's a player of the opposing team
	// within 1000 units
	for (TActorIterator<AMultiCharacter> It(World); It; ++It)
	{
		AMultiCharacter* PlayerCharacter = *It;
		FVector			 CharacterLocation = PlayerCharacter->GetActorLocation();

		// if there's a player from an opposing team within 1k units of the player start, make it ineligible for respawning
		for (int i = UnOccupiedStartPoints.Num() - 1; i >= 0; i--)
		{
			APlayerStart* PlayerStart = UnOccupiedStartPoints[i];
			FVector		  PlayerStartLocation = PlayerStart->GetActorLocation();
			float		  Distance = FVector::Distance(PlayerStartLocation, CharacterLocation);

			if (PlayerCharacter->GetTeam() == EMultiTeam::Red && PlayerStart->PlayerStartTag == FName(TEXT("Blue")))
			{
				if (Distance <= 1000.0f)
				{
					// remove from array
					UnOccupiedStartPoints.Remove(PlayerStart);
				}
			}
			else if (PlayerCharacter->GetTeam() == EMultiTeam::Blue && PlayerStart->PlayerStartTag == FName(TEXT("Red")))
			{
				if (Distance <= 1000.0f)
				{
					// remove from array
					UnOccupiedStartPoints.Remove(PlayerStart);
				}
			}
		}

		for (int i = OccupiedStartPoints.Num() - 1; i >= 0; i--)
		{
			APlayerStart* PlayerStart = OccupiedStartPoints[i];
			FVector		  PlayerStartLocation = PlayerStart->GetActorLocation();
			float		  Distance = FVector::Distance(PlayerStartLocation, CharacterLocation);

			if (PlayerCharacter->GetTeam() == EMultiTeam::Red && PlayerStart->PlayerStartTag == FName(TEXT("Blue")))
			{
				if (Distance <= 1000.0f)
				{
					// remove from array
					OccupiedStartPoints.Remove(PlayerStart);
				}
			}
			else if (PlayerCharacter->GetTeam() == EMultiTeam::Blue && PlayerStart->PlayerStartTag == FName(TEXT("Red")))
			{
				if (Distance <= 1000.0f)
				{
					// remove from array
					OccupiedStartPoints.Remove(PlayerStart);
				}
			}
		}
	}

	if (FoundPlayerStart == nullptr)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
		else if (BackupStartPoints.Num() > 0)
		{
			FoundPlayerStart = BackupStartPoints[FMath::RandRange(0, BackupStartPoints.Num() - 1)];
		}
	}
	return FoundPlayerStart;
}
