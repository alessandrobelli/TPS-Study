// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"

#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create the sphere collision first and make it the root
    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SphereCollision->SetSphereRadius(13.0f);
    SphereCollision->SetCollisionProfileName("Projectile");
	SphereCollision->SetSimulatePhysics(true);
	    SphereCollision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
    RootComponent = SphereCollision;


    
    // Create the static mesh as a child of the sphere collision
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(SphereCollision);
    ProjectileMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // Adjust as needed
    
    // Create the projectile movement component
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
    ProjectileMovement->UpdatedComponent = SphereCollision; // Important: Update the sphere, not the mesh
    ProjectileMovement->InitialSpeed = 1000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;


	// Set the default decal material
	BulletDecalToSpawn = LoadObject<UMaterialInterface>(nullptr, TEXT("Material'/Game/Blueprints/Decals/bullethole2_Mat.bullethole2_Mat'"));
	
    // Add AIPerceptionStimuliSource if needed
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
     // Get the hit location and normal from the hit result
        FVector HitLocation = Hit.Location;
        FVector HitNormal = Hit.Normal;
        
        // Create rotation from the hit normal vector
        FRotator DecalRotation = HitNormal.Rotation();
        
        // Spawn parameters
        UDecalComponent* BulletHoleDecal = UGameplayStatics::SpawnDecalAttached(
            BulletDecalToSpawn,           // The decal material
            FVector(5.0f, 5.0f, 5.0f),    // Decal size
            OtherComp,                    // Component to attach to
            NAME_None,                    // Socket name (None in this case)
            HitLocation,                  // Location
            DecalRotation,                // Rotation
            EAttachLocation::KeepWorldPosition, // Location type
            7.0f                          // Lifespan (0 = infinite)
        );

	Destroy();
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

