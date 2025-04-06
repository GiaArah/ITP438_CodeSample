// Copywright 2023 Gia Lang

#include "AmmoPickup.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiCharacter.h"

// Sets default values
AAmmoPickup::AAmmoPickup()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(25.0f);
	SetRootComponent(SphereComp);

	TextRenderComp = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComp"));
	TextRenderComp->SetText(FText::FromString("Ammo"));
	TextRenderComp->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	TextRenderComp->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
	TextRenderComp->SetTextRenderColor(FColor::Green);
	TextRenderComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAmmoPickup::HandleOnBeginOverlap);
	}
}

void AAmmoPickup::HandleOnBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMultiCharacter* OtherChar = Cast<AMultiCharacter>(OtherActor);
	if (OtherChar)
	{
		OtherChar->OnAmmoPickup(AmmoAmountGiven);
		Destroy();
	}
}

void AAmmoPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAmmoPickup, AmmoAmountGiven);
}

// Called every frame
void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
