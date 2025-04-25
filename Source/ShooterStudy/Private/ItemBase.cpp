// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h" // Include if using Data Tables
#include "Kismet/GameplayStatics.h" // For logging, etc.
// Include your character/inventory component headers if needed for interaction logic

// Sets default values
AItemBase::AItemBase()
{
    // Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false; // Usually items don't need to tick constantly

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    // Example: Bind overlap events if you want pickup-on-overlap
    // CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemBase::OnOverlapBegin);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Mesh usually doesn't need collision if the sphere handles interaction

    // Default ItemID if using Data Tables
    // ItemID = FName("DefaultItemID");
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
    Super::BeginPlay();

    /* Optional: Load data from DataTable on BeginPlay
    if (ItemDataTable && ItemID != NAME_None)
    {
        LoadDataFromTable();
    }
    */

    // Potentially update mesh/materials based on loaded ItemData here
}

/* Optional: Data Table Loading Logic
void AItemBase::LoadDataFromTable()
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("AItemBase::LoadDataFromTable - ItemDataTable is not set for %s"), *GetName());
        return;
    }

    FItemData* FoundData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("ItemDataLookup"));
    if (FoundData)
    {
        ItemData = *FoundData; // Copy the data from the table row to our instance
        UE_LOG(LogTemp, Log, TEXT("Loaded data for %s from DataTable row %s"), *GetName(), *ItemID.ToString());

        // Example: Update Mesh based on data (if mesh path stored in FItemData)
        // if(ItemData.WorldMesh) // Assuming FItemData has a TSoftObjectPtr<UStaticMesh> WorldMesh;
        // {
        //     MeshComponent->SetStaticMesh(ItemData.WorldMesh.LoadSynchronous());
        // }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AItemBase::LoadDataFromTable - Could not find Row %s in DataTable %s for Actor %s"),
            *ItemID.ToString(), *ItemDataTable->GetName(), *GetName());
    }
}

#if WITH_EDITOR
void AItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Auto-load data if ItemID or ItemDataTable is changed in the editor details panel
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AItemBase, ItemID) || PropertyName == GET_MEMBER_NAME_CHECKED(AItemBase, ItemDataTable))
    {
        LoadDataFromTable();
    }
}
#endif
*/
