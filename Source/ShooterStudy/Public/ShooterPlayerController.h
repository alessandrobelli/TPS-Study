// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h" // Input
#include "AttributeComponent.h" // Components
#include "GateComponent.h"       // Components
#include "Components/AudioComponent.h" // Components
#include "ShooterPlayerController.generated.h"

// Forward declarations
class UAttributeComponent;
class UGateComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class AWeaponBase;
class AProjectileBase;
class USoundBase;
class UAudioComponent;

UCLASS()
class SHOOTERSTUDY_API AShooterPlayerController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterPlayerController();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called from Blueprint or other external systems if needed
	UFUNCTION(BlueprintCallable)
	void FortniteShootCpp();

	// --- PROPERTIES ---

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Input)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* JumpAction;

    // Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Animation)
	UAnimMontage* ShootingMontage;

	// Weapon & Firing
	UPROPERTY(EditAnywhere, Category= Firing)
    FName WeaponSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Firing)
	TSubclassOf<class AWeaponBase> DefaultWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= Firing)
	AWeaponBase* CurrentWeaponCpp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= Firing)
	TSubclassOf<class AProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = Firing)
    float WeaponBPM = 300.f;

	UPROPERTY(VisibleAnywhere, Category = Firing)
	float FireDelay = 60.0f / WeaponBPM;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon")
	int CurrentAmmo = 40;

	// Status
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bAlive = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bCrouching = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bFiring = false; 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bReloading = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bCanInteract = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bIsHealing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Status")
	bool bWeaponBlockingHit = false; 

	// Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bImmortal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bShootDebug = false;
	

	// Sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
	UAudioComponent* ShepardTone;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to assign the initial mapping context
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Called when health changes
	UFUNCTION()
	void OnPlayerGetHit(UAttributeComponent* OwningComp, float NewHealth, float Delta);

    // --- COMPONENTS ---
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAttributeComponent* AttributeComponent;

	// --- PROTECTED HELPERS ---
	void SpawnWeapon();


private:
    // --- COMPONENTS ---
	UPROPERTY(VisibleAnywhere, Category = "Components")
    UGateComponent* GateComponent;

    // --- INPUT HANDLERS ---
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
    void ShootWeaponAction(); // Bound to FireAction Started
	void StopShootWeapon();   // Bound to FireAction Canceled/Completed

	// --- FIRING LOGIC ---
	UFUNCTION() // UFUNCTION needed for Timer Delegate and Gate Delegate
	void FireWeapon(); // Called by GateComponent OnGateEntered
	void ActualFireCode(); // The core firing implementation
	void CalculateNearbyObstacles(FHitResult& Hit, const FVector& WeaponStart, const FRotator& WeaponRot) const; // Helper for FortniteShootCpp

    // --- GATE LOGIC ---
	UFUNCTION() // UFUNCTION needed for Timer Delegate
	void EnterGate() const; // Called by RetriggerableDelay timer

    // --- INTERNAL STATE / TIMERS ---
    FTimerHandle RetriggerableDelayTimerHandle; // Used by FireWeapon timer loop
    bool bIsHoldingFire = false; // Tracks if the fire input is held (Consider renaming or removing if GateComponent handles this)
	int32 UUIDLatentAction = FMath::Rand(); // Used for RetriggerableDelay LatentActionInfo

	// --- UTILITY ---
	static float Dot(const FVector3d& A, const FVector3d& B); // Helper for FortniteShootCpp

};