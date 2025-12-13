// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ItemBase.h"
#include "ProjectileBase.h"
#include "ShooterCharacterInventory.h"
#include "WeaponBase.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AShooterPlayerController::AShooterPlayerController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	InitializeCameras();

    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponentCpp"));
    InteractionComponent = CreateDefaultSubobject<UShooterCharacterInventory>(TEXT("InteractionComponentCpp"));

	ShepardTone = CreateDefaultSubobject<UAudioComponent>("ShepardTone");
	ShepardTone->bAutoActivate = false;
	USoundBase* ShepardToneSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/Sounds/shepard_tone.shepard_tone'"));
	ShepardTone->SetSound(ShepardToneSound);
	EmptyRifleSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/Sounds/EmptyRifle.EmptyRifle'"));
	
	WeaponSocketName = FName(TEXT("hand_rSocket"));

	 // Create the gate component
    GateComponent = CreateDefaultSubobject<UGateComponent>(TEXT("GateComponent"));
	
    WeaponBPM = 60.0f;
}

void AShooterPlayerController::StopShootWeapon()
{
	GateComponent->Close();
}

// Called when the game starts or when spawned
void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

    // --- Determine which curves to use (Editor assigned or C++ default) ---
    UCurveFloat* OffsetCurveToUse = AimingCameraOffsetCurve; // Start with the editor-assigned property
    UCurveFloat* FOVCurveToUse = AimingCameraFOVCurve;       // Start with the editor-assigned property

    // Check if the Offset curve needs a C++ default
    if (!OffsetCurveToUse)
    {
        
        CppCreatedOffsetCurve = NewObject<UCurveFloat>(this, TEXT("CppDefaultOffsetCurve"));
        if (CppCreatedOffsetCurve)
        {
            // Make sure defaults were stored before this point!
            CppCreatedOffsetCurve->FloatCurve.AddKey(0.0f, DefaultCameraSocketOffset.X);
            CppCreatedOffsetCurve->FloatCurve.AddKey(0.1f, 60.0f); 
            OffsetCurveToUse = CppCreatedOffsetCurve;
        }
    }

    // Check if the FOV curve needs a C++ default
    if (!FOVCurveToUse)
    {
         // Create a default curve in C++ if none was assigned in the editor
         CppCreatedFOVCurve = NewObject<UCurveFloat>(this, TEXT("CppDefaultFOVCurve"));
         if (CppCreatedFOVCurve)
         {
            // Make sure defaults were stored before this point!
            CppCreatedFOVCurve->FloatCurve.AddKey(0.0f, DefaultCameraFOV);
            CppCreatedFOVCurve->FloatCurve.AddKey(0.1f, 75.0f); 
            FOVCurveToUse = CppCreatedFOVCurve; 
         }
    }
    // --- Curves decided ---


    // --- Setup Aiming Timeline USING the determined curves ---
    // Now check if we have *any* valid curve to use for the timeline
    if (OffsetCurveToUse || FOVCurveToUse)
    {
        FOnTimelineFloat OffsetProgressDelegate;
        FOnTimelineFloat FOVProgressDelegate;

        OffsetProgressDelegate.BindUFunction(this, FName("AimingTimelineUpdate_Offset"));
        FOVProgressDelegate.BindUFunction(this, FName("AimingTimelineUpdate_FOV"));

        // Add tracks using the potentially C++ created curves
        if (OffsetCurveToUse) // Use the variable holding the curve we decided on
        {
            AimingCameraTimeline.AddInterpFloat(OffsetCurveToUse, OffsetProgressDelegate, FName("AimingOffset"));
            UE_LOG(LogTemp, Log, TEXT("Added Offset curve to timeline: %s"), *GetNameSafe(OffsetCurveToUse)); 
        }
        if (FOVCurveToUse) // Use the variable holding the curve we decided on
        {
            AimingCameraTimeline.AddInterpFloat(FOVCurveToUse, FOVProgressDelegate, FName("AimingFOV"));
             UE_LOG(LogTemp, Log, TEXT("Added FOV curve to timeline: %s"), *GetNameSafe(FOVCurveToUse)); 
        }
    }
    else
    {
        // This warning should now only appear if BOTH editor assignment AND C++ creation failed
        UE_LOG(LogTemp, Warning, TEXT("No valid aiming curves (editor-assigned or C++ default) found for %s. Timeline visuals disabled."), *GetName());
    }
    // --- Timeline Setup Complete ---

	if (APlayerCameraManager* PlayerCameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager; PlayerCameraManager != nullptr)
	{
		PlayerCameraManager->ViewPitchMin = -30.0f;
		PlayerCameraManager->ViewPitchMax = 30.0f;
	}


	AttributeComponent->OnHealthChanged.AddDynamic(this, &AShooterPlayerController::OnPlayerGetHit);
	
	// Bind to gate entered event
	GateComponent->OnGateEntered.AddDynamic(this, &AShooterPlayerController::FireWeapon);
    GateComponent->bStartClosed = false;
	
	// Create the weapon actor in the world so it can be rendered
	CurrentWeaponCpp = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeapon, FVector::ZeroVector, FRotator::ZeroRotator);
	if (CurrentWeaponCpp)
	{
	    // Hide it initially until properly attached
	    CurrentWeaponCpp->SetActorHiddenInGame(true);
	}
	SpawnWeapon();
	
}

void AShooterPlayerController::OnPlayerGetHit(UAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (NewHealth <= 0 && !bImmortal)
	{
		bAlive = false;
		USkeletalMeshComponent* currentMesh = GetMesh();
		currentMesh->SetSimulatePhysics(true);
		currentMesh->SetPhysicsBlendWeight(1.f);
		currentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		DisableInput(GetWorld()->GetFirstPlayerController());
		//TODO: restart level
	}else
	{
		if (Delta < 0)
		{
			check(HitSound);
			UGameplayStatics::PlaySound2D(this, HitSound);
			
		}
	}
}



// Called every frame
void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimingCameraTimeline.TickTimeline(DeltaTime);

}

// Called to bind functionality to input
void AShooterPlayerController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PlayerController->Player))
	{
		if (InputMappingContext != nullptr)
		{
			UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
			InputSystem->AddMappingContext(InputMappingContext, 0);
			
		}
        
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterPlayerController::ShootWeaponAction);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this, &AShooterPlayerController::StopShootWeapon);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterPlayerController::StopShootWeapon);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterPlayerController::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Started, this, &AShooterPlayerController::StartAiming);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Canceled, this, &AShooterPlayerController::StopAiming);
		EnhancedInputComponent->BindAction(AimingAction, ETriggerEvent::Completed, this, &AShooterPlayerController::StopAiming);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AShooterPlayerController::CheckForInteractable);
	}

}

void AShooterPlayerController::SpawnWeapon()
{
    if (!CurrentWeaponCpp || !GetMesh())
    {
        UE_LOG(LogTemp, Error, TEXT("Missing weapon or character mesh"));
        return;
    }
    
    // Unhide the weapon now that we'll attach it
    CurrentWeaponCpp->SetActorHiddenInGame(false);
    
    if (USkeletalMeshComponent* WeaponMesh = CurrentWeaponCpp->GetMesh())
    {
        WeaponMesh->SetVisibility(true);
        
        if (USkeletalMeshSocket const* RightHandSocket = GetMesh()->GetSocketByName(WeaponSocketName))
        {
            if (RightHandSocket->AttachActor(CurrentWeaponCpp, GetMesh()))
            {
                UE_LOG(LogTemp, Warning, TEXT("Weapon attached to right hand socket"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to attach weapon to right hand socket"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Weapon mesh is null"));
    }
}

void AShooterPlayerController::CalculateNearbyObstacles(FHitResult& Hit, const FVector& WeaponStart, const FRotator& WeaponRot) const
{
	FVector WeaponEnd = UKismetMathLibrary::GetForwardVector(WeaponRot);
	WeaponEnd *= 150.f;
	WeaponEnd += WeaponStart;
	FCollisionQueryParams TraceParams;

	TraceParams.AddIgnoredActor(this);  // Ignore the character firing the weapon
	TraceParams.bTraceComplex = true;   // For more accurate collision detection
	TraceParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WeaponStart, WeaponEnd, ECollisionChannel::ECC_Camera, TraceParams);

	if (bShootDebug)
	{
		DrawDebugLine(GetWorld(), WeaponStart, WeaponEnd, FColor::Red, false, 1.f, 0, 1.f);
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Green, false, 1.0f);
		}
	}

	if (!bHit)
	{
		Hit.Init();  // Clear old data when no obstacle detected
	}
}


/** FIRE ACTIONS */

// 4. Fortnite rifle shooting code. 
void AShooterPlayerController::FortniteShootCpp()
{
	check(CurrentWeaponCpp);
	if (CurrentAmmo < 1)
	{
		
		if (ConcurrencySettings)
		{
			ConcurrencySettings->Concurrency.MaxCount = 1;
		    ConcurrencySettings->Concurrency.ResolutionRule = EMaxConcurrentResolutionRule::StopOldest;
		}
		if (EmptyRifleSound != nullptr)
		{
		    UGameplayStatics::PlaySoundAtLocation(this, EmptyRifleSound, GetActorLocation(), 0.8f,1,0,nullptr, ConcurrencySettings);
		}
		return;
	}
	CurrentAmmo--;

	// play the animation on the weapon
	CurrentWeaponCpp->GetMesh()->PlayAnimation(CurrentWeaponCpp->FireAnimation, false);
		
	FHitResult FirstHit;
	FHitResult ActualHit;
	FRotator WeaponRot;
	FVector WeaponStart;
	FVector ActualWeaponTraceStart;
	
	// Manually placed name of the muzzle socket
	FName ShootSocket = "Shoot";
	CurrentWeaponCpp->GetMesh()->GetSocketWorldLocationAndRotation(ShootSocket, WeaponStart, WeaponRot);
	CalculateNearbyObstacles(FirstHit, WeaponStart, WeaponRot);
	FVector CameraLocationStart;
	FRotator CameraRotation;

	Cast<APlayerController>(GetController())->GetPlayerViewPoint(CameraLocationStart, CameraRotation);
	FVector CameraForwardVectorEnd = CameraRotation.Vector() * 15000.f;
	ActualWeaponTraceStart = WeaponStart;

	// if there's nothing directly in front of the muzzle, use the camera location, otherwise use the muzzle location
	if (!FirstHit.bBlockingHit || (FirstHit.ImpactPoint == FVector::ZeroVector))
	{
		ActualWeaponTraceStart = CameraLocationStart;
	}
	
	FCollisionQueryParams TraceParams;
	GetWorld()->LineTraceSingleByChannel(ActualHit, ActualWeaponTraceStart, CameraForwardVectorEnd, ECollisionChannel::ECC_Visibility, TraceParams);
	if (bShootDebug) DrawDebugLine(GetWorld(), ActualWeaponTraceStart, CameraForwardVectorEnd, FColor::Green, false, 1.f, 0, 1.f);

	FVector ProjectileStartingLocation;

	// Only validate hit direction if we actually hit something
	if (ActualHit.bBlockingHit)
	{
		// Check if hit point is in front of the character
		FVector CharacterForward = GetActorForwardVector();
		FVector HitDirection = ActualHit.ImpactPoint - GetActorLocation();
		HitDirection = HitDirection.GetSafeNormal();

		float DotProduct = Dot(CharacterForward, HitDirection);

		// If dot product is positive, hit is in front of character
		// When the camera hit is behind the character, it means we want to shoot directly from the weapon
		// When the camera hit is in front, we want to shoot from the camera
		if (DotProduct > 0.0f)
		{
			ProjectileStartingLocation = ActualWeaponTraceStart;
			UE_LOG(LogTemp, Warning, TEXT("Hit is in front of character (Dot: %f)"), DotProduct);
		}
		else
		{
			ProjectileStartingLocation = WeaponStart;
			UE_LOG(LogTemp, Warning, TEXT("Hit is behind character (Dot: %f)"), DotProduct);
		}
	}
	else
	{
		// No hit, just use the appropriate start position (camera if muzzle clear, weapon otherwise)
		ProjectileStartingLocation = ActualWeaponTraceStart;
	}

	// we need to set manually the Shooting montage for the player
	check(ShootingMontage);

	// 1. Play animation montage for the player shooting
	if (ShootingMontage)
	{
		PlayAnimMontage(ShootingMontage);
	}
	
	// 2. Spawn a projectile at ProjectileStartingLocation
	check(ProjectileClass);
	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		
		FRotator ProjectileRotation = (ActualHit.bBlockingHit ? (ActualHit.TraceEnd - ProjectileStartingLocation).Rotation() : CameraRotation);
		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, ProjectileStartingLocation, ProjectileRotation, SpawnParams);
		
		// 3. Apply Velocity to the projectile
		if (Projectile)
		{
			FVector LaunchDirection = ProjectileRotation.Vector();
			// Add character velocity to projectile to avoid shooting inside weapon when moving
			FVector CharacterVelocity = GetVelocity();
			FVector ProjectileVelocity = (LaunchDirection * 20000.0f) + CharacterVelocity; // Adjust speed as needed
			
			// Apply the velocity to the projectile (needs implementation in ProjectileBase)
			Projectile->ProjectileMovement->Velocity = ProjectileVelocity;
		}
	}
}
	
// 3. Actual Fire code, executed in the delay
void AShooterPlayerController::ActualFireCode()
{
	// this is where I call fortniteshootcpp and eventually helldivershootcpp
	FortniteShootCpp();
	UE_LOG(LogTemp, Warning, TEXT("Weapon fired! Current ammo: %d"), CurrentAmmo);
	
}


// 2. Fire weapon called from Gate Enter Delegate
// Retriggerable delay and FireDelay helps to fire the weapon at a certain Bullet x Minute rate
void AShooterPlayerController::FireWeapon()
{
	ActualFireCode();
    // Create the latent action info
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.UUID = UUIDLatentAction;
    LatentInfo.Linkage = 0;
	// if we could place directly GateComponent->Enter() this would change 
    LatentInfo.ExecutionFunction = FName("EnterGate");

    // Call the retriggerable delay - from UE5 counts down and triggers it's output link when the time remaining falls to zero
    UKismetSystemLibrary::RetriggerableDelay(this, FireDelay, LatentInfo);
}

// Callback function needed for the retriggerable delay
void AShooterPlayerController::EnterGate() const
{
	// Call the gate component's Enter function, which will trigger the FireWeapon function
	GateComponent->Enter();
}

// --- INPUT HANDLERS ---
// 1. Shoot weapon action
void AShooterPlayerController::ShootWeaponAction() 
{
	if (bReloading || bIsHealing) return;

	// Check if the gate is open before firing
	if (!GateComponent->IsGateOpen()) GateComponent->Open();
	// Call the gate component's Enter function, which will trigger the FireWeapon function
	GateComponent->Enter();
	
}

void AShooterPlayerController::CheckForInteractable()
{
    FVector CameraLocationStart;
    FRotator CameraRotation;
    FHitResult HitResult;

    Cast<APlayerController>(GetController())->GetPlayerViewPoint(CameraLocationStart, CameraRotation);
    FVector CameraForwardVectorEnd = CameraLocationStart + (CameraRotation.Vector() * 1500.f); // Reduced range for precision

    // Set up trace parameters
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    // Perform trace
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocationStart, CameraForwardVectorEnd, ECC_Visibility, QueryParams);

    // Process what was hit
    if (bHit && HitResult.GetActor())
    {
        UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s, Class: %s"), 
               *HitResult.GetActor()->GetName(), *HitResult.GetActor()->GetClass()->GetName());
        InteractionComponent->AddItem(HitResult);

    }
    else if (bInteractionDebug)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hit detected"));
    }
}

void AShooterPlayerController::InitializeCameras()
{
    // Create the spring arm
    CameraBoomCpp = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

    CameraBoomCpp->SetupAttachment(RootComponent);
    
    CameraBoomCpp->bUsePawnControlRotation = true;
	CameraBoomCpp->bDoCollisionTest = false;
    // Set boom properties
    CameraBoomCpp->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f)); // Example: Position boom slightly above center
    CameraBoomCpp->TargetArmLength = 400.0f;
    CameraBoomCpp->SocketOffset = FVector(0.0f, 100.0f, 75.0f);


    // Create the third-person camera
    ThirdPersonCameraCpp = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCameraCpp->SetupAttachment(CameraBoomCpp, USpringArmComponent::SocketName); // Attach camera to end of boom
    ThirdPersonCameraCpp->bUsePawnControlRotation = false; // Camera itself shouldn't rotate with controller relative to boom
    //ThirdPersonCameraCpp->SetRelativeLocation(FVector(-70.0f, 0.0f, 20.0f)); // Often (0,0,0) relative to boom socket is fine


    // Create the first-person camera
    FirstPersonCameraCpp = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));


    // Set first person camera relative transform relative to the 'head' socket
    FirstPersonCameraCpp->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Adjust as needed
    FirstPersonCameraCpp->SetRelativeRotation(FRotator(0.f, 90.f, 0.f)); 
    FirstPersonCameraCpp->bUsePawnControlRotation = true; // Crucial: FP camera should directly use controller rotation

    // Set initial activation state
    FirstPersonCameraCpp->SetAutoActivate(false); // Start deactivated
    ThirdPersonCameraCpp->SetActive(true);        // Start activated
    ThirdPersonCameraCpp->SetAutoActivate(true);  // Ensure it's active by default

}

void AShooterPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AShooterPlayerController::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
}


// --- Aiming Functions ---

void AShooterPlayerController::StartAiming()
{
	UE_LOG(LogTemp, Log, TEXT("StartAiming"));
	bIsAiming = true; // Assumes bool bIsAiming; exists in your header

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = AimingWalkSpeed;
	}

if (CameraBoomCpp)
{
	UE_LOG(LogTemp, Log, TEXT("CameraBoomCpp is valid, setting socket offset"));
	CameraBoomCpp->SocketOffset = FVector(0.f, 75.f, 100.f);
}
else
{
	UE_LOG(LogTemp, Warning, TEXT("CameraBoomCpp is nullptr! Check if it's being overridden in Blueprint or destroyed"));
	// Try to find the component by name/class if the direct reference is lost
	TArray<USpringArmComponent*> SpringArms;
	GetComponents<USpringArmComponent>(SpringArms);
	if (SpringArms.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Found %d spring arm component(s)"), SpringArms.Num());
		CameraBoomCpp = SpringArms[0]; // Reassign the reference
	}
}

// Similar safety check for ThirdPersonCameraCpp
if (ThirdPersonCameraCpp)
{
	UE_LOG(LogTemp, Log, TEXT("ThirdPersonCameraCpp is valid"));
}
else
{
	UE_LOG(LogTemp, Warning, TEXT("ThirdPersonCameraCpp is nullptr! Check if it's being overridden in Blueprint or destroyed"));
	// Try to find the component by name/class if the direct reference is lost
	TArray<UCameraComponent*> Cameras;
	GetComponents<UCameraComponent>(Cameras);
	if (Cameras.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Found %d camera component(s)"), Cameras.Num());
		// Find the third-person camera (assuming it's the one attached to the spring arm)
		for (UCameraComponent* Camera : Cameras)
		{
			if (Camera->GetAttachParent() == CameraBoomCpp)
			{
				ThirdPersonCameraCpp = Camera;
				UE_LOG(LogTemp, Log, TEXT("Found ThirdPersonCameraCpp attached to CameraBoom"));
				break;
			}
		}
		// If we couldn't find the camera attached to spring arm, just use the first one
		if (!ThirdPersonCameraCpp && Cameras.Num() > 0)
		{
			ThirdPersonCameraCpp = Cameras[0];
			UE_LOG(LogTemp, Log, TEXT("Using first camera as ThirdPersonCameraCpp"));
		}
	}
}

	AimingCameraTimeline.PlayFromStart();
}

void AShooterPlayerController::StopAiming()
{
	UE_LOG(LogTemp, Log, TEXT("StopAiming"));
    // Only stop if we were actually aiming (prevents issues if Completed fires without Started)
    if (!bIsAiming)
    {
        return;
    }
    
    bIsAiming = false;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // Restore default speed only if it's valid
        if (DefaultWalkSpeed > 0) // Basic check
        {
             MoveComp->MaxWalkSpeed = DefaultWalkSpeed;
        }
        else // Fallback if default wasn't captured
        {
            MoveComp->MaxWalkSpeed = 500.0f; // Or your known default
            UE_LOG(LogTemp, Warning, TEXT("DefaultWalkSpeed was invalid, setting MaxWalkSpeed to fallback value."));
        }

    }

    AimingCameraTimeline.Reverse();
}

// --- Timeline Update Functions ---

void AShooterPlayerController::AimingTimelineUpdate_Offset(float Value)
{
    // Add Log to see if this function is called and what value it receives
    UE_LOG(LogTemp, Log, TEXT("Timeline Update Offset: Value = %f"), Value);
	
    if (CameraBoomCpp)
    {
        CameraBoomCpp->SocketOffset = FVector(Value, 75.f, 100.f);
    }
}

void AShooterPlayerController::AimingTimelineUpdate_FOV(float Value)
{
    UE_LOG(LogTemp, Log, TEXT("Timeline Update FOV: Value = %f"), Value);
    if (ThirdPersonCameraCpp)
    {
        // Check if this is the active camera component on this actor
        bool bIsActiveCam = ThirdPersonCameraCpp->IsActive();
        UE_LOG(LogTemp, Log, TEXT("Attempting SetFieldOfView on %s. IsActive = %s"), *ThirdPersonCameraCpp->GetName(), bIsActiveCam ? TEXT("TRUE") : TEXT("FALSE"));

        ThirdPersonCameraCpp->SetFieldOfView(Value);
    }
     else { UE_LOG(LogTemp, Warning, TEXT("ThirdPersonCameraCpp is NULL in FOV update!")); }
}
// --- UTILITY ---

// Basic Dot Product implementation
inline float AShooterPlayerController::Dot(const FVector& A, const FVector& B)
{
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}