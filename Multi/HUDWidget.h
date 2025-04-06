// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiPlayerState.h"
#include "Components/HorizontalBox.h"
#include "HUDWidget.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEditableTextBoxCommittedEvent, const FText&, Text, ETextCommit::Type, CommitMethod);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSendReadyStateDelegate, bool, bIsReady);


UCLASS()
class MULTI_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void		 DisplayTextAlert(const FString& Text, FLinearColor TextColor, float TextDuration);
	void		 HideAlertText();
	void		 ToggleGrenadeText();
	void		 ToggleDeathText(bool bIsVisible);
	void		 AddChatMessage(EMultiTeam ChatTeam, const FString& SenderName, const FString& Message);
	void		 FocusChatTeam();
	void		 FocusChatAll();
	void		 SetChatEntryVisibility(ESlateVisibility NewVisibility) { ChatEntry->SetVisibility(NewVisibility); }
	class UCheckBox* GetCheckBox() { return ReadyCheckBox.Get(); }
	void UpdateTagImage(int TagIndex);
	void ToggleSaveThrobber(bool bIsVisible);

protected:
	bool bIsAllChat = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> AlertTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> RedScoreTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> BlueScoreTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> AmmoCountTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> GrenadeTextBlock;

	// chat widgets

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DeathTextBlock;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> ChatBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> ChatMessages;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> ChatEntry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Team;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> ChatTextBox;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatMessageWidget> ChatMessageWidgetClass;

	UPROPERTY(BlueprintAssignable, Category = "TextBox|Event")
	FOnEditableTextBoxCommittedEvent OnTextCommitted;

	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleOnCheckStateChanged(bool bIsChecked);

	// start game widgets

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCheckBox> ReadyCheckBox;

	UPROPERTY(BlueprintAssignable)
	FSendReadyStateDelegate OnReadyStateChanged;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Timer;

	float DeathTextTimer = 5.0f;

	// Tag Decal Widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> TagImage;

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UMaterialInterface>> TagMaterials;

	// Save System Widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCircularThrobber> AutoSaveThrobber;

};
