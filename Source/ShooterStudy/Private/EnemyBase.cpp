// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyBase.h"
#include "AttributeComponent.h"
#include "TimerManager.h"

AEnemyBase::AEnemyBase()
{
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponentCpp"));
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	AttributeComponent->OnHealthChanged.AddDynamic(this, &AEnemyBase::OnEnemyHealthChanged);
}

void AEnemyBase::OnEnemyHealthChanged(UAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (NewHealth <= 0.f && bAlive)
	{
		bAlive = false;
		ReactToDeath();
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AEnemyBase::DestroySelf, FMath::Max(0.0f, DestroyDelay), false);
	}
}

void AEnemyBase::DestroySelf()
{
	Destroy();
}
