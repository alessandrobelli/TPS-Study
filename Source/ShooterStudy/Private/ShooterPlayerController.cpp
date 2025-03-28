// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ProjectileBase.h"
#include "VectorTypes.h"
#include "WeaponBase.h"
#include "Engine/SkeletalMeshSocket.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AShooterPlayerController::AShooterPlayerController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));


	ShepardTone = CreateDefaultSubobject<UAudioComponent>("ShepardTone");
	ShepardTone->bAutoActivate = false;
	USoundBase* ShepardToneSound = LoadObject<USoundBase>(nullptr, TEXT("SoundWave'/Game/Sounds/shepard_tone.shepard_tone'"));
	ShepardTone->SetSound(ShepardToneSound);
	
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
if (bShootDebug) DrawDebugLine(GetWorld(), WeaponStart, WeaponEnd, FColor::Red, false, 1.f, 0, 1.f);
if (!bHit)
{
    if (bShootDebug) DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Green, false, 1.0f);
}
	if (bShootDebug)DrawDebugLine(GetWorld(), WeaponStart, WeaponEnd, FColor::Red, false, 1.f, 0, 1.f);
}





/** FIRE ACTIONS */

// 4. Fortnite rifle shooting code. 
void AShooterPlayerController::FortniteShootCpp()
{
	check(CurrentWeaponCpp);
	if (CurrentAmmo < 1)
	{
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

	// Check if hit point is in front of the character
	FVector CharacterForward = GetActorForwardVector();
	FVector HitDirection = ActualHit.ImpactPoint - GetActorLocation();
	HitDirection = HitDirection.GetSafeNormal();
			
	float DotProduct = Dot(CharacterForward, HitDirection);
	FVector ProjectileStartingLocation;
	
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


// --- UTILITY ---

// Basic Dot Product implementation
inline float AShooterPlayerController::Dot(const FVector& A, const FVector& B)
{
	return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}