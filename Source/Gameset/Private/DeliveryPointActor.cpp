#include "DeliveryPointActor.h"
#include "DeliveryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ADeliveryPointActor::ADeliveryPointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	DeliverySphere = CreateDefaultSubobject<USphereComponent>(TEXT("DeliverySphere"));
	DeliverySphere->SetupAttachment(RootComponent);
	DeliverySphere->SetSphereRadius(150.f);
	DeliverySphere->SetCollisionProfileName(TEXT("Trigger"));
}

void ADeliveryPointActor::BeginPlay()
{
	Super::BeginPlay();
	DeliverySphere->OnComponentBeginOverlap.AddDynamic(this, &ADeliveryPointActor::OnSphereBeginOverlap);
	DeliverySphere->OnComponentEndOverlap.AddDynamic(this,   &ADeliveryPointActor::OnSphereEndOverlap);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADeliveryPointActor::ActivateDropoff(const FString& OrderID)
{
	BoundOrderID = OrderID;
	bIsActive    = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ADeliveryPointActor::DeactivateDropoff()
{
	BoundOrderID = FString();
	bIsActive    = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADeliveryPointActor::OnSphereBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (!bIsActive) return;
	if (UDeliveryComponent* DC = OtherActor ? OtherActor->FindComponentByClass<UDeliveryComponent>() : nullptr)
		DC->AddNearbyDropoff(this);
}

void ADeliveryPointActor::OnSphereEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UDeliveryComponent* DC = OtherActor ? OtherActor->FindComponentByClass<UDeliveryComponent>() : nullptr)
		DC->RemoveNearbyDropoff(this);
}
