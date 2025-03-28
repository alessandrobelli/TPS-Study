// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

/*
 * This component handles the health attribute of an actor.
 * Accepts the owner actor as a parameter and provides a delegate to notify when health changes.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged , UAttributeComponent*, OwningComp, float, NewHealth, float, Delta);


UCLASS( ClassGroup=(Custom), meta=(DisplayName="Attributes Component",BlueprintSpawnableComponent) )
class SHOOTERSTUDY_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();

	UFUNCTION(BlueprintCallable)
	void ApplyHealthChange(float Delta);

	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth();
	
	UFUNCTION(BlueprintCallable)
	bool IsAlive();

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	float Health;
	float MaxHealth;

public:	
	
		
};
