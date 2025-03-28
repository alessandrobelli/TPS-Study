// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Mesh");
	SetRootComponent(Mesh);

	FireAnimation = CreateDefaultSubobject<UAnimationAsset>(TEXT("Fire Animation"));

	FireAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Weapons/MilitaryWeapSilver/Weapons/Animations/Fire_Rifle_W"));

}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AWeaponBase::PlayFireAnimation() const
{
    	Mesh->PlayAnimation(FireAnimation, false);
	    return true;
}
