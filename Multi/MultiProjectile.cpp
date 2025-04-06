// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "MultiCharacter.h"
#include "MultiGameStateBase.h"
#include "GameFramework/GameMode.h"
#include "Containers/UnrealString.h"
#include "MultiGameMode.h"

AMultiProjectile::AMultiProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AMultiProjectile::OnHit); // set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AMultiProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		Destroy();
	}

	AMultiCharacter* OtherMultiActor = Cast<AMultiCharacter>(OtherActor);
	if (OtherMultiActor)
	{
		AMultiCharacter* MultiActor = Cast<AMultiCharacter>(GetInstigator());

		// if other actor is on opposing team, increase instigator team score by 1
		if (OtherMultiActor->GetTeam() != MultiActor->GetTeam())
		{
			// only add points if the match is actually in progress
			if (Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode())->GetMatchState() != MatchState::InProgress)
			{
				return;
			}
			// only add points if the character is alive
			else if (!OtherMultiActor->GetIsAlive())
			{
				return;
			}

			// if the other actor is on the opposing team and still alive, update scores and kill other actor
			OtherMultiActor->KillPlayer();

			AMultiPlayerState* OtherPlayerState = OtherMultiActor->GetPlayerState<AMultiPlayerState>();
			if (OtherPlayerState && OtherPlayerState->GetKillStreak() >= 3)
			{
				AMultiGameStateBase* MultiGameState = Cast<AMultiGameStateBase>(GetWorld()->GetGameState());
				FString				 Message = FString::Printf(TEXT("%s ended %s's streak!"), *MultiActor->GetName(), *OtherMultiActor->GetName());
				FLinearColor		 MessageColor;
				if (MultiActor->GetTeam() == EMultiTeam::Red)
				{
					MessageColor = FLinearColor::Red;
				}
				else
				{
					MessageColor = FLinearColor::Blue;
				}
				MultiGameState->MulticastTextAlert(Message, MessageColor, 2.5f);
			}

			// add score to the actor that shot the other actor
			AMultiPlayerState* MultiActorState = Cast<AMultiPlayerState>(MultiActor->GetPlayerState());
			if (MultiActorState)
			{
				MultiActorState->SetScore(MultiActorState->GetScore() + 1.0f);
				if (HasAuthority())
				{
					MultiActorState->SetKillStreak(MultiActorState->GetKillStreak() + 1);
				}
			}

			// update game state scores
			AMultiGameStateBase* MultiGameState = Cast<AMultiGameStateBase>(GetWorld()->GetGameState());
			if (MultiGameState)
			{
				if (MultiActor->GetTeam() == EMultiTeam::Red)
				{
					MultiGameState->RedTeamScore += 1;
				}
				else
				{
					MultiGameState->BlueTeamScore += 1;
				}
			}

			// send out kill streak alerts
			FLinearColor MessageColor;
			FString		 Message;
			if (MultiActorState->Team == EMultiTeam::Red)
			{
				MessageColor = FLinearColor::Red;
			}
			else
			{
				MessageColor = FLinearColor::Blue;
			}
			if (MultiActorState->GetKillStreak() == 3)
			{
				Message = FString::Printf(TEXT("%s is on a killing spree!"), *MultiActorState->GetPlayerName());
				MultiGameState->MulticastTextAlert(Message, MessageColor, 2.5f);
			}
			else if (MultiActorState->GetKillStreak() == 5)
			{
				Message = FString::Printf(TEXT("%s is dominating!"), *MultiActorState->GetPlayerName());
				MultiGameState->MulticastTextAlert(Message, MessageColor, 2.5f);
			}
			else if (MultiActorState->GetKillStreak() == 7)
			{
				Message = FString::Printf(TEXT("%s is UNSTOPPABLE!"), *MultiActorState->GetPlayerName());
				MultiGameState->MulticastTextAlert(Message, MessageColor, 2.5f);
			}
			else if (MultiActorState->GetKillStreak() == 9)
			{
				Message = FString::Printf(TEXT("%s is GODLIKE!"), *MultiActorState->GetPlayerName());
				MultiGameState->MulticastTextAlert(Message, MessageColor, 2.5f);
			}

			Destroy();
		}
		else
		{
			Destroy();
		}
	}
}