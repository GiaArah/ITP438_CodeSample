// Copywright 2023 Gia Lang

#include "FriendButtonWidget.h"
#include "Components/Button.h"
#include "MultiOnlineSystem.h"
#include "Blueprint/UserWidget.h"

void UFriendButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UFriendButtonWidget::HandleJoinButtonClick);
	}
}

void UFriendButtonWidget::HandleJoinButtonClick()
{
	// call join friend session on multionlinesubsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiOnlineSystem* Subsystem = GameInstance->GetSubsystem<UMultiOnlineSystem>();
		if (Subsystem)
		{
			Subsystem->JoinFriendSession(FriendIndex);
		}
	}
}
