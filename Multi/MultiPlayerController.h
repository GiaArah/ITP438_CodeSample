// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiPlayerController.generated.h"

/**
 *
 */
UCLASS()
class MULTI_API AMultiPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void SendTextAlertData(const FString& Text, FLinearColor TextColor, float TextDuration);

	void ToggleDeathText(bool bIsVisible);

	void			  CallTeamChatFocus();
	void			  CallAllChatFocus();
	class UHUDWidget* GetHUDWidgetInstance() { return HUDWidgetInstance.Get(); }

	UFUNCTION(Server, Reliable)
	void ServerApplyTag(int TagIndex);

protected:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class ATagActor>> TagActorClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<class UHUDWidget> HUDWidgetInstance;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(Client, Reliable)
	void ClientOnPossess();

	FTimerHandle AlertTextTimerHandle;
};
