// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

/**
 *
 */

DECLARE_DELEGATE_FourParams(FReadFriendsListCompleteDelegate, int32, bool, const FString&, const FString&);

UCLASS()
class MULTI_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> RefreshButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> FriendBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> HostNewGameButton;

	virtual void NativeConstruct() override;

	FReadFriendsListCompleteDelegate MyDelegate;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UFriendButtonWidget> FriendButtonWidgetClass;

	UFUNCTION()
	void HandleRefreshButtonClick();

	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
};
