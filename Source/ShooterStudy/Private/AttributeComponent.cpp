// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeComponent.h"

// Sets default values for this component's properties
UAttributeComponent::UAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
}

void UAttributeComponent::ApplyHealthChange(float Delta)
{
	float NewHealth = Health + Delta;
	if (NewHealth > MaxHealth)
	{
		NewHealth = MaxHealth;
	}
	else if (NewHealth < 0)
	{
		NewHealth = 0;
	}
	Health = NewHealth;

	//TODO: test if this is called and works
	OnHealthChanged.Broadcast(this, Health, Delta);
}

float UAttributeComponent::GetCurrentHealth()
{
	return Health;
}

bool UAttributeComponent::IsAlive()
{
	return (Health > 0);
}


// Called when the game starts
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

