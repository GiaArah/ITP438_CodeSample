// Copywright 2023 Gia Lang

#include "Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystem.h"
#include "Multicharacter.h"
#include "MultiGameStateBase.h"
#include "MultiPlayerState.h"

// Sets default values
AGrenade::AGrenade()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetCapsuleSize(5.0f, 10.0f);
	CapsuleComp->SetCollisionProfileName(FName(TEXT("Projectile")));

	RootComponent = CapsuleComp;
	// USceneComponent* UpdatedRoot = CreateDefaultSubobject<USceneComponent>(TEXT("UpdatedComponent"));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>((TEXT("ProjectileMovement")));
	// ProjectileMovement->SetupAttachment(UpdatedRoot);
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->ProjectileGravityScale = 1.5f;
}

void AGrenade::LaunchGrenade()
{
	GetWorldTimerManager().SetTimer(ExplosionTimer, this, &AGrenade::OnExplosion, 3.0f, false);
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
}

void AGrenade::MulticastExplosionEffects_Implementation()
{
	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, GetActorLocation());//(ExplodeSound, CapsuleComp);
	}

	if (ExplodeParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticle, GetActorLocation(), GetActorRotation(), true);
	}
}

void AGrenade::OnExplosion()
{
	MulticastExplosionEffects();
	CheckCollisions();
	GetWorld()->GetTimerManager().SetTimer(DestroyGrenadeTimer, this, &AGrenade::DestroyGrenade, 0.2f, false);
}

void AGrenade::CheckCollisions()
{
	FVector			ExplosionLocation = GetActorLocation();
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMultiCharacter::StaticClass(), Players);

	for (AActor* Player : Players)
	{
		if (AMultiCharacter* Chara = Cast<AMultiCharacter>(Player))
		{
			FVector PlayerLocation = Chara->GetActorLocation();
			float	Distance = FVector::Dist2D(ExplosionLocation, PlayerLocation);

			if (Distance <= 250.0f)
			{
				Chara->KillPlayer();

				if (Chara->GetTeam() != GrenadeTeam)
				{
					AMultiGameStateBase* GS = Cast<AMultiGameStateBase>(GetWorld()->GetGameState());
					if (GS)
					{
						if (GrenadeTeam == EMultiTeam::Red)
						{
							GS->RedTeamScore = GS->RedTeamScore + 1;
						}
						else
						{
							GS->BlueTeamScore = GS->BlueTeamScore + 1;
						}
					}
				}
			}
		}
	}
}

void AGrenade::DestroyGrenade()
{
	Destroy();
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
