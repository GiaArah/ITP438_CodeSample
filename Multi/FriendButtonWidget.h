// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "FriendButtonWidget.generated.h"

UCLASS()
class MULTI_API UFriendButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void		 SetFriendName(FText NewName) { FriendName.Get()->SetText(NewName); }
	void		 SetFriendIndex(int NewIndex) { FriendIndex = NewIndex; }
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> FriendName;

	UFUNCTION()
	void HandleJoinButtonClick();

	int FriendIndex;
};
