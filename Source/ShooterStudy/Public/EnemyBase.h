// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UAttributeComponent;

UCLASS(Blueprintable)
class SHOOTERSTUDY_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnEnemyHealthChanged(UAttributeComponent* OwningComp, float NewHealth, float Delta);

	// Fires once when health first reaches <= 0. Override point for Blueprint VFX/SFX/anim reactions.
	UFUNCTION(BlueprintImplementableEvent, Category = "Status")
	void ReactToDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttributeComponent* AttributeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	bool bAlive = true;

	// Delay between ReactToDeath() and actually destroying the actor, so death VFX/animation has time
	// to play instead of the actor vanishing the instant ReactToDeath() is invoked. 0 = next tick.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float DestroyDelay = 0.0f;

private:
	FTimerHandle DestroyTimerHandle;
	void DestroySelf();
};
