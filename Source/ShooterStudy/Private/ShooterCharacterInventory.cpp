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
void UShooterCharacterInventory::AddItem(const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Display, TEXT("Adding item"));
	// check if actor hit is an item
	if (AItemBase* Item = Cast<AItemBase>(HitResult.GetActor()))
		{
					// check if item is not already in inventory
					if (!InventoryMap.Contains(Item->ItemData.Name))
					{
						// add item to inventory
						InventoryMap.Add(Item->ItemData.Name, Item);
						Item->Destroy();
						UE_LOG(LogTemp, Warning, TEXT("Picked up item: %s"), *Item->ItemData.Name);
					}
					else
					{
						// already have this item in inventory, increment amount
						AItemBase* ExistingItem = InventoryMap[Item->ItemData.Name];
						// if no amount, add 1
						if (ExistingItem->ItemData.Amount == 0)
						{
							ExistingItem->ItemData.Amount = 1;
						}else
						{
							ExistingItem->ItemData.Amount += Item->ItemData.Amount;	
						}
						
					}
		}else
			{
					UE_LOG(LogTemp, Error, TEXT("Failed to cast to AItemBase"));
			}
}

