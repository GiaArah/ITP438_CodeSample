// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ChatMessageWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UChatMessageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetChannel(FText ChannelText, FLinearColor ChannelColor) 
	{ 
		Channel->SetText(ChannelText); 
		Channel->SetColorAndOpacity(ChannelColor);
	}
	void SetSender(FText SenderText) { Sender->SetText(SenderText); }
	void SetMessage(FText MessageText) { Message->SetText(MessageText); }

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Channel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Sender;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Message;
	
};
