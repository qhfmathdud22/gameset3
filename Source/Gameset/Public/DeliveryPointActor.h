#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeliveryTypes.h"
#include "DeliveryPointActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOrderDelivered, AActor*, OwnerActor, float, Reward);

// 배달 완료 지점 (고객 위치)
UCLASS()
class GAMESET_API ADeliveryPointActor : public AActor
{
	GENERATED_BODY()

public:
	ADeliveryPointActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* DeliverySphere;

public:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FString BoundOrderID;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsActive = false;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnOrderDelivered OnOrderDelivered;

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void ActivateDropoff(const FString& OrderID);

	UFUNCTION(BlueprintCallable, Category = "Delivery")
	void DeactivateDropoff();

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
