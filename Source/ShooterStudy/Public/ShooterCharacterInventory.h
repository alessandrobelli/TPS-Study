// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterCharacterInventory.generated.h"

class AItemBase;

UCLASS(DisplayName="AC_InventoryCpp", Category="Inventory")
class SHOOTERSTUDY_API UShooterCharacterInventory : public UActorComponent
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere)
	AItemBase* CurrentItem;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

		// Sets default values for this actor's properties
	UShooterCharacterInventory();

	void PickUpItem();

	void AddItem(const FHitResult& HitResult);

	// add a map of items that represent the inventory: string name and ItemBase
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TMap<FString, class AItemBase*> InventoryMap;

};
