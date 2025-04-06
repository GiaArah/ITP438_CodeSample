// Copywright 2023 Gia Lang

#pragma once

#include "CoreMinimal.h"
#include "MultiPlayerState.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class MULTI_API AGrenade : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGrenade();
	void	   LaunchGrenade();
	void	   SetGrenadeTeam(EMultiTeam Team) { GrenadeTeam = Team; }
	EMultiTeam GetGrenadeTeam() { return GrenadeTeam; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastExplosionEffects();

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundBase> ExplodeSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UParticleSystem> ExplodeParticle;

	FTimerHandle ExplosionTimer;
	FTimerHandle DestroyGrenadeTimer;
	void		 OnExplosion();

	void CheckCollisions();
	void DestroyGrenade();

	EMultiTeam GrenadeTeam = EMultiTeam::None;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
