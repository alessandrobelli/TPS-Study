// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    IC_None         UMETA(DisplayName = "None"),
    IC_Weapon       UMETA(DisplayName = "Weapon"),
    IC_Armor        UMETA(DisplayName = "Armor"),
    IC_Consumable   UMETA(DisplayName = "Consumable"),
};


USTRUCT(BlueprintType) // Makes this struct available to Blueprints
struct FItemData : public FTableRowBase // Inherit from FTableRowBase if you plan to use this in Data Tables
{
    GENERATED_BODY()

public:
    // The name of the item
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FString Name;

    // The description text for the item
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    FText Description;

    // The category this item belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
    EItemCategory Category;

    // The quantity or stack amount of the item (if applicable)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (ClampMin = "0")) // Added meta for non-negative amount
    int32 Amount;

    // Default constructor (optional but good practice)
    FItemData()
        : Name(TEXT(""))
        , Description(FText::GetEmpty())
        , Category(EItemCategory::IC_None) // Default category
        , Amount(0)
    {}
};

// Forward declarations
class USphereComponent;
class UStaticMeshComponent;
class UDataTable; // Include if you plan to load data from Data Tables

UCLASS(Abstract, Blueprintable) // Abstract: Prevents placing this base class directly in the level. Blueprintable: Allows creating Blueprint subclasses.
class SHOOTERSTUDY_API AItemBase : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AItemBase();

    // --- Components ---

    // Collision component to detect interaction (e.g., overlap or trace hits)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent; 

    // Visual representation of the item in the world
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    // --- Item Data ---

    // Holds the core information about this item (Name, Description, Category, etc.)
    // EditDefaultsOnly is often suitable here, meaning you set it in the Blueprint Class Defaults,
    // not per-instance in the level. Use EditAnywhere if instances need unique base data.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties")
    FItemData ItemData;

    /* --- Optional: Data Table Integration ---
    // If you want to define items primarily in a DataTable

    // The Data Table containing item definitions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties | Data Table")
    UDataTable* ItemDataTable;

    // The specific row in the ItemDataTable that defines this item
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties | Data Table")
    FName ItemID; // Corresponds to the Row Name in the DataTable
    */


protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    /* Optional: Helper to load data from DataTable
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Item Properties | Data Table") // CallInEditor allows running from editor button
    void LoadDataFromTable();

    // Override PostEditChangeProperty to auto-load data when ItemID changes in editor
    #if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    #endif
    */

};