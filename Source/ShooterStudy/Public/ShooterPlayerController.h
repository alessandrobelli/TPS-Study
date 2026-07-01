// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h" // Input
#include "AttributeComponent.h" // Components
#include "GateComponent.h"       // Components
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h" // Components
#include "Components/TimelineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ShooterPlayerController.generated.h"

class UShooterCharacterInventory;
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

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoomCpp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCameraCpp = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= Camera)
	UCameraComponent* FirstPersonCameraCpp = nullptr;


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input, meta=(AllowPrivateAccess = true))
	UInputAction* AimingAction;

	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int CurrentAmmo = 40;

	// Idle weapon sway (hip-fire only; disabled while aiming)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing | Sway")
	float WeaponSwayAmplitude = 2.0f; // degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firing | Sway")
	float WeaponSwaySpeed = 1.5f;

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
	bool bIsAiming = false;
	
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
	USoundBase* EmptyRifleSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Sounds")
	USoundConcurrency* ConcurrencySettings = nullptr;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShooterCharacterInventory* InteractionComponent;
	
	// --- PROTECTED HELPERS ---
	void SpawnWeapon();

	// Functions bound to Aiming Input Action (Triggered/Started and Completed)
    void StartAiming();
    void StopAiming();
	UFUNCTION() 
	void AimingTimelineUpdate_Offset(float Value);
	UFUNCTION() 
	void AimingTimelineUpdate_FOV(float Value);

	// Timeline object used for smooth transitions
    FTimeline AimingCameraTimeline;

    // Curve assets (Create these in UE Editor and assign in BP Defaults)
    UPROPERTY(EditDefaultsOnly, Category = "Aiming | Timeline")
    UCurveFloat* AimingCameraOffsetCurve; 

    UPROPERTY(EditDefaultsOnly, Category = "Aiming | Timeline")
    UCurveFloat* AimingCameraFOVCurve; 

	UPROPERTY()
	UCurveFloat* CppCreatedOffsetCurve;

	UPROPERTY()
	UCurveFloat* CppCreatedFOVCurve;
	
    // Store default values to return to
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    float DefaultWalkSpeed = 500.0f; // Default walk speed before aiming

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    FVector DefaultCameraSocketOffset = FVector(0.0f, 100.0f, 75.0f); // Default socket offset before aiming

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    float DefaultCameraFOV = 90.0f; // Default camera FOV before aiming
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    float AimingWalkSpeed = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    float AimingSocketOffsetY = 100.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Aiming | Config")
    float AimingSocketOffsetZ = 75.0f;

	/** Distance to trace forward for interaction checks. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
    float InteractionDistance = 300.0f;

    /** Performs a line trace to check for interactable items in front of the player. */
    void CheckForInteractable();

    // Optional: Add a boolean for debug drawing
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Debug")
    bool bInteractionDebug = true;



private:
    // --- COMPONENTS ---
	UPROPERTY(VisibleAnywhere, Category = "Components")
    UGateComponent* GateComponent;

	void InitializeCameras();

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

	// --- WEAPON SWAY ---
	void UpdateWeaponSway(); // Called every Tick; tiny idle rotation on CurrentWeaponCpp when not aiming

    // --- GATE LOGIC ---
	UFUNCTION() // UFUNCTION needed for Timer Delegate
	void EnterGate() const; // Called by RetriggerableDelay timer

    // --- INTERNAL STATE / TIMERS ---
    FTimerHandle RetriggerableDelayTimerHandle; // Used by FireWeapon timer loop
    bool bIsHoldingFire = false; // Tracks if the fire input is held
	int32 UUIDLatentAction = FMath::Rand(); // Used for RetriggerableDelay LatentActionInfo

	// --- UTILITY ---
	static float Dot(const FVector3d& A, const FVector3d& B); // Helper for FortniteShootCpp

};