// Copywright 2023 Gia Lang

#include "HUDWidget.h"
#include "MultiPlayerController.h"
#include "MultiPlayerState.h"
#include "MultiCharacter.h"
#include "MultiGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "ChatMessageWidget.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Components/Image.h"
#include "Components/CircularThrobber.h"
#include "MultiGameMode.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChatTextBox->OnTextCommitted.AddDynamic(this, &UHUDWidget::HandleTextCommitted);
	ReadyCheckBox->OnCheckStateChanged.AddDynamic(this, &UHUDWidget::HandleOnCheckStateChanged);
}

void UHUDWidget::DisplayTextAlert(const FString& Text, FLinearColor TextColor, float TextDuration)
{
	if (AlertTextBlock)
	{
		if (AlertTextBlock->GetVisibility() != ESlateVisibility::Visible)
		{
			AlertTextBlock->SetVisibility(ESlateVisibility::Visible);
		}
		AlertTextBlock->SetText(FText::FromString(Text));
		AlertTextBlock->SetColorAndOpacity(TextColor);
	}
}

void UHUDWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	ChatTextBox->SetText(FText::FromString(FString(TEXT(""))));
	AMultiPlayerController* OwningPlayerController = GetOwningPlayer<AMultiPlayerController>();
	if (OwningPlayerController)
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(OwningPlayerController);
	}

	if (CommitMethod == ETextCommit::OnEnter)
	{
		AMultiPlayerState* PlayerState = GetOwningPlayerState<AMultiPlayerState>();
		if (PlayerState)
		{
			EMultiTeam MessageTeam;
			// if supposed to be for everyone
			if (bIsAllChat)
			{
				MessageTeam = EMultiTeam::None;
			}
			else
			{
				// if team only
				MessageTeam = PlayerState->Team;
			}
			PlayerState->ServerSendChatMessage(MessageTeam, PlayerState->GetPlayerName(), Text.ToString());
		}
	}
	ChatEntry->SetVisibility(ESlateVisibility::Hidden);
}

void UHUDWidget::HandleOnCheckStateChanged(bool bIsChecked)
{
	AMultiPlayerState* PS = Cast<AMultiPlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		 PS->SendReadyToStart(bIsChecked);
	}
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AMultiCharacter*		MyChar = Cast<AMultiCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	AMultiPlayerController* MyPC = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	AMultiPlayerState*		PlayerState = MyPC ? Cast<AMultiPlayerState>(MyPC->PlayerState) : nullptr;
	AMultiGameStateBase*	GameState = GetWorld()->GetGameState<AMultiGameStateBase>();

	FName MatchState = GameState->GetMatchState();
	if (GameState && MatchState == MatchState::WaitingToStart)
	{
		if (Timer->GetVisibility() != ESlateVisibility::HitTestInvisible)
		{
			Timer->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		FString TimerText = FString::Printf(TEXT("%f"), GameState->WaitingToStartTime);
		Timer->SetText(FText::FromString(TimerText));
	}
	else if (DeathTextTimer > 0.0f && DeathTextBlock->GetVisibility() == ESlateVisibility::Visible)
	{
		if (Timer->GetVisibility() != ESlateVisibility::HitTestInvisible)
		{
			Timer->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		FString TimerText = FString::Printf(TEXT("%f"), DeathTextTimer);
		Timer->SetText(FText::FromString(TimerText));
		DeathTextTimer -= InDeltaTime;
		if (DeathTextTimer <= 0.0f)
		{
			ToggleDeathText(false);
		}
	}
	else
	{
		if (Timer->GetVisibility() != ESlateVisibility::Hidden)
		{
			Timer->SetVisibility(ESlateVisibility::Hidden);
		}
		if (ReadyCheckBox->GetVisibility() != ESlateVisibility::Hidden)
		{
			ReadyCheckBox->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (PlayerState && GameState && PlayerState->Team == EMultiTeam::Red)
	{
		RedScoreTextBlock->SetText(FText::FromString(FString::Printf(TEXT("RED %i (%i)"), GameState->RedTeamScore, static_cast<int>(PlayerState->GetScore()))));
		BlueScoreTextBlock->SetText(FText::FromString(FString::Printf(TEXT("BLUE %i"), GameState->BlueTeamScore)));
	}
	else if (PlayerState && GameState && PlayerState->Team == EMultiTeam::Blue)
	{
		BlueScoreTextBlock->SetText(FText::FromString(FString::Printf(TEXT("BLUE %i (%i)"), GameState->BlueTeamScore, static_cast<int>(PlayerState->GetScore()))));
		RedScoreTextBlock->SetText(FText::FromString(FString::Printf(TEXT("RED %i"), GameState->RedTeamScore)));
	}

	if (MyChar)
	{
		AmmoCountTextBlock->SetText(FText::FromString(FString::Printf(TEXT("AMMO %i"), MyChar->GetAmmoCount())));
	}
}

void UHUDWidget::HideAlertText()
{
	if (AlertTextBlock)
	{
		AlertTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHUDWidget::ToggleGrenadeText()
{
	if (GrenadeTextBlock && GrenadeTextBlock->GetVisibility() == ESlateVisibility::Visible)
	{
		GrenadeTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		GrenadeTextBlock->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHUDWidget::ToggleDeathText(bool bIsVisible)
{
	if (bIsVisible)
	{
		DeathTextBlock->SetVisibility(ESlateVisibility::Visible);
		DeathTextTimer = 5.0f;
	}
	else
	{
		DeathTextBlock->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHUDWidget::AddChatMessage(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message)
{
	UChatMessageWidget* NewMessage = NewObject<UChatMessageWidget>(GetWorld(), ChatMessageWidgetClass);
	ChatMessages->AddChild(NewMessage);

	if (ChatTeam == EMultiTeam::None)
	{
		NewMessage->SetChannel(FText::FromString(TEXT("[ALL]")), FLinearColor::White);
	}
	else if (ChatTeam == EMultiTeam::Red)
	{
		NewMessage->SetChannel(FText::FromString(TEXT("[TEAM]")), FLinearColor::Red);
	}
	else
	{
		NewMessage->SetChannel(FText::FromString(TEXT("[TEAM]")), FLinearColor::Blue);
	}
	NewMessage->SetSender(FText::FromString(SenderName));
	NewMessage->SetMessage(FText::FromString(Message));
}

void UHUDWidget::FocusChatTeam()
{
	AMultiPlayerController* OwningPlayerController = GetOwningPlayer<AMultiPlayerController>();
	if (OwningPlayerController)
	{
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(OwningPlayerController, this);
	}
	Team->SetText(FText::FromString(FString(TEXT("[TEAM]"))));
	bIsAllChat = false;
}

void UHUDWidget::FocusChatAll()
{
	AMultiPlayerController* OwningPlayerController = GetOwningPlayer<AMultiPlayerController>();
	if (OwningPlayerController)
	{
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(OwningPlayerController, this);
	}
	Team->SetText(FText::FromString(FString(TEXT("[ALL]"))));
	bIsAllChat = true;
}

void UHUDWidget::UpdateTagImage(int TagIndex)
{
	TagImage->SetBrushFromMaterial(TagMaterials[TagIndex]);
}

void UHUDWidget::ToggleSaveThrobber(bool bIsVisible)
{
	if (bIsVisible)
	{
		AutoSaveThrobber->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		AutoSaveThrobber->SetVisibility(ESlateVisibility::Hidden);
	}
}
