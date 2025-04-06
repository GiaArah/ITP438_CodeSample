// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiCharacter.h"
#include "MultiProjectile.h"
#include "Grenade.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "TP_WeaponComponent.h"
#include "MultiPlayerState.h"
#include "MultiCharacter.h"
#include "MultiPlayerController.h"
#include "MultiGameMode.h"
#include "MultiGameStateBase.h"
#include "HUDWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

//////////////////////////////////////////////////////////////////////////
// AMultiCharacter

AMultiCharacter::AMultiCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	// Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	// Create a mesh component that will be used when being viewed from a '3rd person' view
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetupAttachment(RootComponent);
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Mesh3P->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));

	// Create the Third Person CameraComponent
	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(Mesh3P);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;
	ThirdPersonCameraComponent->bAutoActivate = false;
	ThirdPersonCameraComponent->SetRelativeLocation(FVector(0.f, -400.f, 350.f)); // Position the camera
	ThirdPersonCameraComponent->SetRelativeRotation(FRotator(-45.f, 90.f, 0.f));

	// Create a mesh component that will be used when being viewed from a '3rd person' view of the gun
	Gun3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh3P"));
	Gun3P->SetOwnerNoSee(true);
	Gun3P->SetupAttachment(Mesh3P, FName(TEXT("GripPoint")));
}

void AMultiCharacter::SetupInputMappingContext()
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AMultiCharacter::SetTeam(EMultiTeam NewTeam)
{
	Team = NewTeam;

	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		OnRep_Team();
	}
}

void AMultiCharacter::AddAmmoCount_Implementation(int Amount)
{
	AmmoCount = AmmoCount + Amount;
}

void AMultiCharacter::BeginPlay()
{
	// Call the base class
	Super::BeginPlay();

	SetupInputMappingContext();

	if (GetLocalRole() == ROLE_Authority && WeaponOnSpawn)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		WeaponActor = GetWorld()->SpawnActor<AActor>(WeaponOnSpawn, GetActorLocation(), FRotator::ZeroRotator, Params);

		UTP_WeaponComponent* WeaponComp = WeaponActor ? WeaponActor->FindComponentByClass<UTP_WeaponComponent>() : nullptr;
		if (WeaponComp)
		{
			WeaponComp->AttachWeapon(this);
		}
	}
}

void AMultiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiCharacter, WeaponActor);
	DOREPLIFETIME(AMultiCharacter, Team);
	DOREPLIFETIME(AMultiCharacter, AmmoCount);
	DOREPLIFETIME(AMultiCharacter, bIsAlive);
	DOREPLIFETIME(AMultiCharacter, ReplicatedControlRotation);
	DOREPLIFETIME(AMultiCharacter, DeathMontage);
}

void AMultiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ReplicatedControlRotation = GetControlRotation();
	}
}

void AMultiCharacter::OnRep_WeaponActor()
{
	UTP_WeaponComponent* WeaponComp = WeaponActor ? WeaponActor->FindComponentByClass<UTP_WeaponComponent>() : nullptr;
	if (WeaponComp)
	{
		WeaponComp->AttachWeapon(this);
	}
}

void AMultiCharacter::OnRep_Team()
{
	FVector ParamValue = FVector(0.0f, 0.0f, 0.0f);
	if (Team == EMultiTeam::Red)
	{
		ParamValue = FVector(1.0f, 0.0f, 0.0f);
	}
	else
	{
		ParamValue = FVector(0.0f, 0.0f, 1.0f);
	}
	Mesh1P->SetVectorParameterValueOnMaterials(FName(TEXT("BodyColor")), ParamValue);
	Mesh3P->SetVectorParameterValueOnMaterials(FName(TEXT("BodyColor")), ParamValue);
}

void AMultiCharacter::OnRep_IsAlive()
{
	if (!bIsAlive)
	{

		if (IsLocallyControlled())
		{
			// change the camera to the third person camera
			Mesh1P->SetOwnerNoSee(true);
			USkeletalMeshComponent* GunMesh = WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
			GunMesh->SetOwnerNoSee(true);
			Mesh3P->SetOwnerNoSee(false);
			Gun3P->SetOwnerNoSee(false);
			FirstPersonCameraComponent->Deactivate();
			ThirdPersonCameraComponent->Activate();

			// display message and set client timer
			AMultiPlayerController* PC = GetController<AMultiPlayerController>();
			PC->ToggleDeathText(true);
		}

		UAnimInstance* AnimInstance = Mesh3P->GetAnimInstance();
		if (AnimInstance)
		{
			float DeathTimer = AnimInstance->Montage_Play(DeathMontage);
			// Call the gameplay timer to trigger the pause montage on death
			float ActivationTime = (DeathTimer - 0.25f);
			GetWorldTimerManager().SetTimer(DeathAnimTimerHandle, this, &AMultiCharacter::PauseMontage, ActivationTime, false);
		}

		UAnimInstance* FirstPersonAnimInstance = Mesh1P->GetAnimInstance();
		if (FirstPersonAnimInstance && GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
		{
			float FirstPersonDeathTimer = FirstPersonAnimInstance->Montage_Play(FirstPersonDeathMontage);
			// Call the gameplay timer to trigger the pause the montage on death
			float ActivationTime = (FirstPersonDeathTimer - 0.25f);
			GetWorldTimerManager().SetTimer(FirstPersonDeathAnimTimerHandle, this, &AMultiCharacter::PauseFirstPersonMontage, ActivationTime, false);
		}

		// server-side collision disable
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMultiCharacter::PauseMontage()
{
	UAnimInstance* AnimInstance = Mesh3P->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Pause();
	}
}

void AMultiCharacter::PauseFirstPersonMontage()
{
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Pause();
	}
}

void AMultiCharacter::Server_SetIsAlive_Implementation(bool NewIsAlive)
{
	bIsAlive = NewIsAlive;
}

void AMultiCharacter::Respawn_Implementation()
{
	AController* OldController = Controller;
	if (Controller)
	{
		Controller->UnPossess();
	}

	AMultiGameMode* GameMode = Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->RestartPlayer(OldController);
	}

	Destroy();
}

//////////////////////////////////////////////////////////////////////////// Input

void AMultiCharacter::UpdateCanFireGrenade()
{
	bCanFireGrenade = true;
	AMultiPlayerController* PC = GetController<AMultiPlayerController>();
	if (PC)
	{
		PC->GetHUDWidgetInstance()->ToggleGrenadeText();
	}
}

void AMultiCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMultiCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMultiCharacter::Look);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMultiCharacter::OnWeaponFireAction);

		// Grenade
		EnhancedInputComponent->BindAction(FireGrenadeAction, ETriggerEvent::Triggered, this, &AMultiCharacter::OnGrenadeFireAction);

		// Chat
		EnhancedInputComponent->BindAction(ChatTeamAction, ETriggerEvent::Triggered, this, &AMultiCharacter::OnChatTeamAction);
		EnhancedInputComponent->BindAction(ChatAllAction, ETriggerEvent::Triggered, this, &AMultiCharacter::OnChatAllAction);

		// Tag Decals
		EnhancedInputComponent->BindAction(PlaceTagAction, ETriggerEvent::Triggered, this, &AMultiCharacter::OnPlaceTag);
		EnhancedInputComponent->BindAction(SwapTag1Action, ETriggerEvent::Triggered, this, &AMultiCharacter::UpdateTagIndex1);
		EnhancedInputComponent->BindAction(SwapTag2Action, ETriggerEvent::Triggered, this, &AMultiCharacter::UpdateTagIndex2);
		EnhancedInputComponent->BindAction(SwapTag3Action, ETriggerEvent::Triggered, this, &AMultiCharacter::UpdateTagIndex3);
		EnhancedInputComponent->BindAction(SwapTag4Action, ETriggerEvent::Triggered, this, &AMultiCharacter::UpdateTagIndex4);
		EnhancedInputComponent->BindAction(SwapTag5Action, ETriggerEvent::Triggered, this, &AMultiCharacter::UpdateTagIndex5);
	}
}

void AMultiCharacter::OnAmmoPickup(int Amount)
{
	AddAmmoCount(Amount);
}

void AMultiCharacter::KillPlayer()
{
	AMultiPlayerState* ActorState = Cast<AMultiPlayerState>(GetPlayerState());

	if (HasAuthority())
	{
		Server_SetIsAlive(false);
		ActorState->SetKillStreak(0);
	}
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (CMC)
	{
		CMC->DisableMovement();
	}

	// client-side disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// call respawn after 5 seconds
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMultiCharacter::Respawn, 5.0f, false);

	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		OnRep_IsAlive();
	}
}

void AMultiCharacter::Move(const FInputActionValue& Value)
{
	if (!bIsAlive)
	{
		return;
	}
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		// add movement
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AMultiCharacter::Look(const FInputActionValue& Value)
{
	if (!bIsAlive)
	{
		return;
	}
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMultiCharacter::OnWeaponFireAction()
{
	if (AmmoCount > 0)
	{
		ServerFire();
	}
}

void AMultiCharacter::OnGrenadeFireAction()
{
	// check if you can fire (timer is 0 / canFireGrenade == true)
	if (bCanFireGrenade)
	{
		ServerGrenadeFire();
		bCanFireGrenade = false;
		GetWorldTimerManager().SetTimer(GrenadeFireCooldownTimer, this, &AMultiCharacter::UpdateCanFireGrenade, 5.0f, false);
		AMultiPlayerController* PC = GetController<AMultiPlayerController>();
		if (PC)
		{
			PC->GetHUDWidgetInstance()->ToggleGrenadeText();
		}
	}
}

void AMultiCharacter::OnChatTeamAction()
{
	AMultiPlayerController* PC = GetController<AMultiPlayerController>();
	if (PC)
	{
		PC->GetHUDWidgetInstance()->SetChatEntryVisibility(ESlateVisibility::Visible);
		PC->CallTeamChatFocus();
	}
}

void AMultiCharacter::OnChatAllAction()
{
	AMultiPlayerController* PC = GetController<AMultiPlayerController>();
	if (PC)
	{
		PC->GetHUDWidgetInstance()->SetChatEntryVisibility(ESlateVisibility::Visible);
		PC->CallAllChatFocus();
	}
}

void AMultiCharacter::OnPlaceTag()
{
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->ServerApplyTag(TagIndex - 1);
	}
}

void AMultiCharacter::UpdateTagIndex1()
{
	TagIndex = 1;
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->GetHUDWidgetInstance()->UpdateTagImage(TagIndex - 1);
	}
}

void AMultiCharacter::UpdateTagIndex2()
{
	TagIndex = 2;
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->GetHUDWidgetInstance()->UpdateTagImage(TagIndex - 1);
	}
}

void AMultiCharacter::UpdateTagIndex3()
{
	TagIndex = 3;
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->GetHUDWidgetInstance()->UpdateTagImage(TagIndex - 1);
	}
}

void AMultiCharacter::UpdateTagIndex4()
{
	TagIndex = 4;
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->GetHUDWidgetInstance()->UpdateTagImage(TagIndex - 1);
	}
}

void AMultiCharacter::UpdateTagIndex5()
{
	TagIndex = 5;
	if (AMultiPlayerController* PC = GetController<AMultiPlayerController>())
	{
		PC->GetHUDWidgetInstance()->UpdateTagImage(TagIndex - 1);
	}
}

void AMultiCharacter::ServerGrenadeFire_Implementation()
{
	// if player is dead, don't fire
	if (!bIsAlive)
	{
		return;
	}
	UTP_WeaponComponent* WeaponComp = WeaponActor ? WeaponActor->FindComponentByClass<UTP_WeaponComponent>() : nullptr;
	if (WeaponComp)
	{
		WeaponComp->Fire(true);
		// set cooldown timer
	}
}

void AMultiCharacter::ServerFire_Implementation()
{
	// if player is dead, don't fire
	if (!bIsAlive)
	{
		return;
	}
	UTP_WeaponComponent* WeaponComp = WeaponActor ? WeaponActor->FindComponentByClass<UTP_WeaponComponent>() : nullptr;
	if (WeaponComp)
	{
		WeaponComp->Fire(false);
		AmmoCount--;
	}
}

void AMultiCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AMultiCharacter::GetHasRifle()
{
	return bHasRifle;
}