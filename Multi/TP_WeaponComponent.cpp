// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_WeaponComponent.h"
#include "MultiCharacter.h"
#include "MultiProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Grenade.h"
#include "MultiplayerState.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	SetIsReplicatedByDefault(true);
}

void UTP_WeaponComponent::Fire(bool bIsGrenade)
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr && GrenadeClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator	   SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

			// Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = Character;

			// Spawn the projectile at the muzzle
			if (bIsGrenade)
			{
				FRotator  GrenadeRotation = FRotator(90.0f, 0.0f, 0.0f);
				AGrenade* SpawnedGrenade = World->SpawnActor<AGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
				SpawnedGrenade->SetActorRotation(GrenadeRotation);
				SpawnedGrenade->LaunchGrenade();

				AMultiPlayerState* MPS = Cast<AMultiPlayerState>(Character->GetPlayerState());
				if (MPS)
				{
					SpawnedGrenade->SetGrenadeTeam(MPS->Team);
				}
			}
			else
			{
				World->SpawnActor<AMultiProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

	MulticastFireEffects();
}

void UTP_WeaponComponent::AttachWeapon(AMultiCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (Character == nullptr)
	{
		return;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character == nullptr)
	{
		return;
	}
}

void UTP_WeaponComponent::MulticastFireEffects_Implementation()
{
	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance1P = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance1P != nullptr)
		{
			AnimInstance1P->Montage_Play(FireAnimation, 1.f);
		}

		// Get the animation object for the third person mesh
		UAnimInstance* AnimInstance3P = Character->GetMesh3P()->GetAnimInstance();
		if (AnimInstance3P != nullptr)
		{
			AnimInstance3P->Montage_Play(FireAnimation3P, 1.f);
		}
	}
}
