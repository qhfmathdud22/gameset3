#include "PackageActor.h"
#include "DeliveryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

APackageActor::APackageActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	InteractSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractSphere"));
	InteractSphere->SetupAttachment(RootComponent);
	InteractSphere->SetSphereRadius(150.f);
	InteractSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void APackageActor::BeginPlay()
{
	Super::BeginPlay();
	InteractSphere->OnComponentBeginOverlap.AddDynamic(this, &APackageActor::OnSphereBeginOverlap);
	InteractSphere->OnComponentEndOverlap.AddDynamic(this,   &APackageActor::OnSphereEndOverlap);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void APackageActor::ActivateOrder(const FDeliveryOrder& NewOrder)
{
	Order        = NewOrder;
	bIsAvailable = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void APackageActor::PickupPackage(UDeliveryComponent* DeliveryComp)
{
	if (!bIsAvailable || !DeliveryComp) return;

	bIsAvailable = false;
	Order.Status = EOrderStatus::PickedUp;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	OnPickedUp.Broadcast(DeliveryComp->GetOwner());
}

void APackageActor::OnSphereBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	if (!bIsAvailable) return;
	if (UDeliveryComponent* DC = OtherActor ? OtherActor->FindComponentByClass<UDeliveryComponent>() : nullptr)
		DC->AddNearbyPackage(this);
}

void APackageActor::OnSphereEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UDeliveryComponent* DC = OtherActor ? OtherActor->FindComponentByClass<UDeliveryComponent>() : nullptr)
		DC->RemoveNearbyPackage(this);
}
