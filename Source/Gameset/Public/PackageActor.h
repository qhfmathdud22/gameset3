#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryTypes.h"
#include "PackageActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPackagePickedUp, AActor*, OwnerActor);

// 도심에 배치되는 배달 픽업 지점
UCLASS()
class GAMESET_API APackageActor : public AActor
{
	GENERATED_BODY()

public:
	APackageActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* InteractSphere;

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Order")
	FDeliveryOrder Order;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsAvailable = false;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPackagePickedUp OnPickedUp;

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void ActivateOrder(const FDeliveryOrder& NewOrder);

	// UDeliveryComponent를 가진 어떤 캐릭터든 호출 가능
	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void PickupPackage(class UDeliveryComponent* DeliveryComp);

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
