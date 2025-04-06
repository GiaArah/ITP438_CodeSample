// Copywright 2023 Gia Lang

#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "FriendButtonWidget.h"
#include "MultiOnlineSystem.h"
#include "MultiSaveGame.h"
#include "MultiSaveSystem.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleRefreshButtonClick);
	}

	MyDelegate.BindUObject(this, &UMainMenuWidget::OnReadFriendsListComplete);

	HandleRefreshButtonClick();

	UMultiSaveSystem* NewSave = GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (NewSave->SaveFilePresent())
	{
		HostButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		HostButton->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenuWidget::HandleRefreshButtonClick()
{
	// clear the currently active friends
	FriendBox->ClearChildren();

	// call read friends on the subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiOnlineSystem* Subsystem = GameInstance->GetSubsystem<UMultiOnlineSystem>();
		if (Subsystem)
		{
			Subsystem->ReadFriendsList(MyDelegate);
		}
	}
}

void UMainMenuWidget::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			UMultiOnlineSystem* Subsystem = GameInstance->GetSubsystem<UMultiOnlineSystem>();
			if (Subsystem)
			{
				TArray<FString>			 FriendsListNames = Subsystem->GetFriendsListNames();
				for (int i = 0; i < FriendsListNames.Num(); i++)
				{
					//UFriendButtonWidget* NewFriendButton = CreateWidget<UFriendButtonWidget>(this, FriendButtonWidgetClass);
					UFriendButtonWidget* NewFriendButton = CreateWidget<UFriendButtonWidget>(this, FriendButtonWidgetClass);

					FriendBox->AddChildToVerticalBox(NewFriendButton);
					FText Name = FText::FromString(FriendsListNames[i]);
					NewFriendButton->SetFriendName(Name);
					NewFriendButton->SetFriendIndex(i);
				}
			}
		}
	}
}
