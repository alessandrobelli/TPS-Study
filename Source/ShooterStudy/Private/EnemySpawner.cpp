// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemySpawner.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/HitResult.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	WaveNumber = 1;
	CurrentSpawnInterval = InitialSpawnInterval;
	OnWaveChanged(WaveNumber);

	// Repeating spawn timer (per-wave interval) + repeating wave-escalation timer.
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnTick, FMath::Max(0.05f, CurrentSpawnInterval), true);
	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &AEnemySpawner::AdvanceWave, FMath::Max(1.0f, WaveDuration), true);
}

void AEnemySpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::SpawnTick()
{
	const int32 NumThisTick = FMath::Max(1, EnemiesPerSpawn);
	for (int32 i = 0; i < NumThisTick; ++i)
	{
		if (AliveEnemies >= MaxAliveEnemies)
		{
			break; // hard cap reached -- never spawn past it
		}
		TrySpawnOne();
	}
}

void AEnemySpawner::AdvanceWave()
{
	++WaveNumber;

	// Escalate: shrink the spawn interval toward the floor, and bump enemies-per-spawn.
	CurrentSpawnInterval = FMath::Max(MinSpawnInterval, CurrentSpawnInterval - SpawnIntervalStep);
	EnemiesPerSpawn = FMath::Min(MaxEnemiesPerSpawn, EnemiesPerSpawn + EnemiesPerSpawnStep);

	// Re-arm the spawn timer at the new (faster) interval.
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnTick, FMath::Max(0.05f, CurrentSpawnInterval), true);

	OnWaveChanged(WaveNumber);
}

void AEnemySpawner::TrySpawnOne()
{
	if (AliveEnemies >= MaxAliveEnemies)
	{
		return;
	}

	const TSubclassOf<ACharacter> ClassToSpawn = PickEnemyClass();
	if (!ClassToSpawn)
	{
		return; // nothing configured to spawn
	}

	FVector SpawnPoint;
	if (!FindSpawnPoint(SpawnPoint))
	{
		return; // no valid reachable point this attempt -> skip this spawn
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ACharacter* Enemy = World->SpawnActor<ACharacter>(ClassToSpawn, SpawnPoint, FRotator::ZeroRotator, SpawnParams))
	{
		++AliveEnemies;
		// Decrement when the enemy is destroyed (its death logic -> Destroy() -> Destroyed).
		Enemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::HandleEnemyDestroyed);
	}
}

bool AEnemySpawner::FindSpawnPoint(FVector& OutPoint) const
{
	UWorld* World = GetWorld();
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return false; // no navmesh -> can't place safely
	}

	// Center the spawn ring on the player (default) or on this spawner.
	FVector Center = GetActorLocation();
	FVector PlayerLoc = Center;
	if (const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		PlayerLoc = PlayerPawn->GetActorLocation();
		if (bSpawnAroundPlayer)
		{
			Center = PlayerLoc;
		}
	}

	// The placed spawner anchors the arena bound below (the player can roam to the edge, but spawns must
	// never leave the arena -- the cooked navmesh can extend past the playable floor).
	const FVector SpawnerLoc = GetActorLocation();

	// Try a few times to find a point that is (a) on the NavMesh, (b) far enough from the player,
	// (c) inside the arena (MaxSpawnDistance from the spawner), and (d) over SOLID floor.
	for (int32 Attempt = 0; Attempt < MaxPlacementTries; ++Attempt)
	{
		FNavLocation NavLoc;
		// Random navigable point within SpawnRadius that is reachable from Center.
		if (!NavSys->GetRandomReachablePointInRadius(Center, SpawnRadius, NavLoc))
		{
			continue;
		}

		// (b) Far enough from the player (2D, so a height difference can't sneak a too-close point through).
		if (FVector::Dist2D(NavLoc.Location, PlayerLoc) < MinDistanceFromPlayer)
		{
			continue;
		}

		// (c) HARD ARENA BOUND -- the "never spawn outside the map" guard. The cooked runtime navmesh can
		// reach past the floor, so a "valid navmesh point" can still be off the arena; reject anything
		// farther than MaxSpawnDistance from the placed (central) spawner.
		if (FVector::Dist2D(NavLoc.Location, SpawnerLoc) > MaxSpawnDistance)
		{
			continue;
		}

		// (d) There must be SOLID static floor directly beneath -- never spawn over a void or off a ledge
		// edge (where the navmesh can overhang). Trace down from just above and spawn on the actual floor.
		const FVector TraceStart = NavLoc.Location + FVector(0.f, 0.f, 150.f);
		const FVector TraceEnd   = NavLoc.Location - FVector(0.f, 0.f, 500.f);
		FHitResult FloorHit;
		FCollisionQueryParams Params(NAME_None, /*bTraceComplex=*/false, this);
		if (!World->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_WorldStatic, Params))
		{
			continue; // no floor under this point -> reject
		}

		OutPoint = FloorHit.ImpactPoint + FVector(0.f, 0.f, SpawnZOffset);
		return true;
	}

	return false; // couldn't satisfy all checks this tick -> skip (better a skipped spawn than an off-map one)
}

TSubclassOf<ACharacter> AEnemySpawner::PickEnemyClass() const
{
	// Sum positive weights of valid entries.
	float TotalWeight = 0.f;
	for (const FEnemySpawnEntry& Entry : EnemyTypes)
	{
		if (Entry.EnemyClass && Entry.Weight > 0.f)
		{
			TotalWeight += Entry.Weight;
		}
	}

	// No weighted roster -> fall back to the single class (placed-spawner back-compat).
	if (TotalWeight <= 0.f)
	{
		return EnemyClass;
	}

	// Roll in [0, TotalWeight) and walk the cumulative weights.
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (const FEnemySpawnEntry& Entry : EnemyTypes)
	{
		if (Entry.EnemyClass && Entry.Weight > 0.f)
		{
			Roll -= Entry.Weight;
			if (Roll <= 0.f)
			{
				return Entry.EnemyClass;
			}
		}
	}

	return EnemyClass; // numeric edge fallback
}

void AEnemySpawner::HandleEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
}
