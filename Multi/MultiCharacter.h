// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MultiPlayerState.h"
#include "MultiCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config = Game)
class AMultiCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** THIRD person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

public:
	AMultiCharacter();

	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P.Get(); }

	void SetupInputMappingContext();

	EMultiTeam GetTeam() const { return Team; }
	void	   SetTeam(EMultiTeam NewTeam);

	int GetAmmoCount() const { return AmmoCount; }

	UFUNCTION(BlueprintCallable)
	void PauseMontage();

	UFUNCTION(BlueprintCallable)
	void PauseFirstPersonMontage();

protected:
	virtual void BeginPlay();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> WeaponOnSpawn;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponActor, Transient)
	AActor* WeaponActor;

	UFUNCTION()
	void OnRep_WeaponActor();

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh3P;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> Gun3P;

	UPROPERTY(Replicated)
	int AmmoCount = 5;

	UFUNCTION()
	void OnRep_Team();

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	EMultiTeam Team;

	UFUNCTION()
	void OnRep_IsAlive();

	UFUNCTION(Server, Reliable)
	void Server_SetIsAlive(bool NewIsAlive);

	FTimerHandle DeathAnimTimerHandle;
	FTimerHandle FirstPersonDeathAnimTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = true;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere)
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimMontage* FirstPersonDeathMontage;

	FTimerHandle RespawnTimerHandle;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FRotator ReplicatedControlRotation;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> ChatTeamAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> ChatAllAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> PlaceTagAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> SwapTag1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> SwapTag2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> SwapTag3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> SwapTag4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputAction> SwapTag5Action;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireGrenadeAction;

	void OnAmmoPickup(int Amount);

	bool GetIsAlive() const { return bIsAlive; }

	void KillPlayer();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void OnWeaponFireAction();
	void OnGrenadeFireAction();

	void OnChatTeamAction();
	void OnChatAllAction();
	void OnPlaceTag();

	void UpdateTagIndex1();
	void UpdateTagIndex2();
	void UpdateTagIndex3();
	void UpdateTagIndex4();
	void UpdateTagIndex5();
	int	 TagIndex = 1;

	UFUNCTION(Server, Reliable)
	void ServerFire();

	UFUNCTION(Server, Reliable)
	void ServerGrenadeFire();

	FTimerHandle GrenadeFireCooldownTimer;
	bool		  bCanFireGrenade = true;
	void		  UpdateCanFireGrenade();

	UFUNCTION(Server, Reliable)
	void AddAmmoCount(int Amount);

	UFUNCTION(Server, Reliable)
	void Respawn();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};
