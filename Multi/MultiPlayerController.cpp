// Copywright 2023 Gia Lang

#include "MultiPlayerController.h"
#include "HUDWidget.h"
#include "MultiPlayerState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "MultiCharacter.h"
#include "TagActor.h"
#include "EngineUtils.h"
#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"

void AMultiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidgetInstance = NewObject<UHUDWidget>(this, HUDWidgetClass);
		HUDWidgetInstance->SetOwningPlayer(this);
		HUDWidgetInstance->AddToViewport();
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, HUDWidgetInstance);
	}
}

void AMultiPlayerController::SendTextAlertData(const FString& Text, FLinearColor TextColor, float TextDuration)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->DisplayTextAlert(Text, TextColor, TextDuration);
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(AlertTextTimerHandle, HUDWidgetInstance.Get(), &UHUDWidget::HideAlertText, TextDuration);
}

void AMultiPlayerController::ToggleDeathText(bool bIsVisible)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->ToggleDeathText(bIsVisible);
	}
}

void AMultiPlayerController::CallTeamChatFocus()
{
	HUDWidgetInstance->FocusChatTeam();
}

void AMultiPlayerController::CallAllChatFocus()
{
	HUDWidgetInstance->FocusChatAll();
}

void AMultiPlayerController::ServerApplyTag_Implementation(int TagIndex)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		FHitResult			  HitResult;
		FVector				  StartPoint = Char->GetActorLocation();
		FVector				  EndPoint = StartPoint + ControlRotation.Vector() * 500.0f;
		FCollisionQueryParams QueryParams;

		// ignore all characters in the world
		for (TActorIterator<ACharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			QueryParams.AddIgnoredActor(*Iter);
		}
		QueryParams.AddIgnoredActor(Char);
		GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Camera, QueryParams);
		if (HitResult.bBlockingHit)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ATagActor* TagActor = GetWorld()->SpawnActor<ATagActor>(TagActorClass[TagIndex], HitResult.Location, FRotator::ZeroRotator, SpawnParameters);
			if (TagActor)
			{
				FVector Axis = FVector::CrossProduct(FVector::XAxisVector, -HitResult.Normal);
				Axis.Normalize();
				float Dot = FVector::DotProduct(FVector::XAxisVector, -HitResult.Normal);
				float Angle = FMath::Acos(Dot);
				FQuat SpawnQuat(Axis, Angle);

				if (Dot < -0.99f)
				{
					SpawnQuat = FQuat(FVector::ZAxisVector, PI);
				}
				FRotator SpawnRotator(SpawnQuat);

				// if we're on the ground, yaw based on the player control rotation yaw
				if (SpawnRotator.Pitch < -45.0f)
				{
					SpawnRotator.Yaw = ControlRotation.Yaw - 90.0f;
				}
				else
				{
					SpawnRotator.Roll += 90.0f;
				}

				TagActor->SetActorRotation(SpawnRotator);

				// save the data
				UMultiSaveSystem* NewSave = GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
				if (NewSave)
				{
					FTagActorSaveData TagSave;
					TagSave.ClassType = TagActorClass[TagIndex];
					TagSave.Transform = TagActor->GetActorTransform();

					NewSave->GetSave()->AddTag(TagSave);
					NewSave->SaveGame(false);
				}
			}
		}
	}
}

void AMultiPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ClientOnPossess();
	AMultiCharacter* MultiPlayerChar = Cast<AMultiCharacter>(InPawn);
	if (Player)
	{
		// update the character's team
		AMultiPlayerState* CurrentPlayerState = this ? Cast<AMultiPlayerState>(this->PlayerState) : nullptr;
		if (CurrentPlayerState->Team == EMultiTeam::Red)
		{
			MultiPlayerChar->SetTeam(EMultiTeam::Red);
		}
		else
		{
			MultiPlayerChar->SetTeam(EMultiTeam::Blue);
		}
	}

	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		MultiPlayerChar->SetupInputMappingContext();
	}
}

void AMultiPlayerController::ClientOnPossess_Implementation()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
}
