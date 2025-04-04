// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class SHOOTERSTUDY_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

UFUNCTION()
	bool PlayFireAnimation() const;

	UFUNCTION(BlueprintCallable, Category="Weapon")
	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetMesh(USkeletalMeshComponent* NewMesh) { Mesh = NewMesh; }
	
	UPROPERTY(EditAnywhere, Category="Weapon")
	UAnimationAsset* FireAnimation;
private:
	UPROPERTY(VisibleAnywhere, Category="Weapon", meta =(AllowPrivateAccess = true))
	USkeletalMeshComponent* Mesh;



};
