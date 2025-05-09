// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacterInventory.h"

#include "ItemBase.h"
#include "ShooterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

class AShooterPlayerController;
// Sets default values
UShooterCharacterInventory::UShooterCharacterInventory(): CurrentItem(nullptr)
{
}

// Called when the game starts or when spawned
void UShooterCharacterInventory::BeginPlay()
{
	Super::BeginPlay();
	
}

void UShooterCharacterInventory::PickUpItem()
{
	// get owner
	AActor* Owner = GetOwner();
	// cast to ShooterPlayerController and get the "third person camera" and camera boom
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(Owner))
	{
		// get the camera boom and third person camera
		UCameraComponent* Camera = PlayerController->ThirdPersonCameraCpp;
		USpringArmComponent* CameraBoom = PlayerController->FindComponentByClass<USpringArmComponent>();
		
		if (Camera && CameraBoom)
		{
			
			FVector CameraLocation = Camera->GetComponentLocation();
			FVector CameraForward = Camera->GetForwardVector() * CameraBoom->TargetArmLength + 500.0f + CameraLocation;

			// get hit result
			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(Owner);
			CollisionParams.bTraceComplex = false;
			CollisionParams.bReturnPhysicalMaterial = true;

			if (bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, CameraForward, ECollisionChannel::ECC_Camera, CollisionParams))
			{
				AddItem(HitResult);
			}
						
		}
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to AShooterPlayerController"));
	}
}

/**
 * Add item to inventory - check if item is already in inventory and increment amount
 * @param HitResult 
 */
// ShooterCharacterInventory.cpp

void UShooterCharacterInventory::AddItem(const FHitResult& HitResult)
{
    UE_LOG(LogTemp, Display, TEXT("Adding item"));
    if (AItemBase* Item = Cast<AItemBase>(HitResult.GetActor()))
    {
        FString ItemName = Item->ItemData.Name;
        int32 AddAmount = (Item->ItemData.Amount > 0) ? Item->ItemData.Amount : 1;

		if (!InventoryMap.Contains(ItemName))
		{
		    FInventoryEntry NewEntry(Item->ItemData, AddAmount);
		    InventoryMap.Add(ItemName, NewEntry);
		    UE_LOG(LogTemp, Warning, TEXT("Picked up new item: %s (x%d)"), *ItemName, AddAmount);
		}
		else
		{
		    FInventoryEntry& Entry = InventoryMap[ItemName];
		    Entry.Amount += AddAmount;
		    UE_LOG(LogTemp, Warning, TEXT("Increased amount of %s to %d"), *ItemName, Entry.Amount);
		}

        Item->Destroy();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast to AItemBase"));
    }
}

