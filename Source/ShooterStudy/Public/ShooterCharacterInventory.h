// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "GameFramework/Actor.h"
#include "ShooterCharacterInventory.generated.h"

class AItemBase;

USTRUCT(BlueprintType)
struct FInventoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemData ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;

    FInventoryEntry() : Amount(0) {}
    FInventoryEntry(const FItemData& InItemData, int32 InAmount) : ItemData(InItemData), Amount(InAmount) {}
};
UCLASS( ClassGroup=(Custom), meta=(DisplayName="AC_InventoryCpp",BlueprintSpawnableComponent) )
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

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void AddItem(const FHitResult& HitResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TMap<FString, FInventoryEntry> InventoryMap;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	TArray<FInventoryEntry> GetInventoryItems() const
	{
	    TArray<FInventoryEntry> Items;
	    InventoryMap.GenerateValueArray(Items);
	    return Items;
	}

};
