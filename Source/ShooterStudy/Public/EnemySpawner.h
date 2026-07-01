// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ACharacter;

/** One weighted entry in the spawner's enemy roster. */
USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
	GENERATED_BODY()

	/** Enemy type (a Character Blueprint) to spawn. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	TSubclassOf<ACharacter> EnemyClass;

	/** Relative weight; higher = rolled more often. <= 0 is ignored. */
	UPROPERTY(EditAnywhere, Category="Spawning", meta=(ClampMin="0.0"))
	float Weight = 1.f;
};

/**
 *  Endless, escalating wave spawner. Place one in the level. Spawns enemies on a
 *  repeating timer at random reachable points on the NavMesh, ramping difficulty
 *  every wave, capped by MaxAliveEnemies so the framerate never tanks.
 */
UCLASS(Blueprintable)
class SHOOTERSTUDY_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:

	AEnemySpawner();

	/** Current alive enemy count (spawned and not yet destroyed). */
	UFUNCTION(BlueprintPure, Category="Spawning")
	int32 GetAliveEnemyCount() const { return AliveEnemies; }

	/** Current wave number (for the HUD wave title). */
	UFUNCTION(BlueprintPure, Category="Spawning")
	int32 GetWaveNumber() const { return WaveNumber; }

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Weighted roster of enemy types to roll among each spawn. Preferred over EnemyClass. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	TArray<FEnemySpawnEntry> EnemyTypes;

	/** Fallback single enemy class if EnemyTypes is empty (back-compat with placed spawners). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	TSubclassOf<ACharacter> EnemyClass;

	/** Spawn interval at wave 1 (s). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float InitialSpawnInterval = 2.0f;

	/** Floor for the spawn interval as waves escalate (s). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float MinSpawnInterval = 0.4f;

	/** Amount the spawn interval shrinks each wave (s). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float SpawnIntervalStep = 0.2f;

	/** Seconds per wave before difficulty escalates. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float WaveDuration = 20.0f;

	/** Hard cap on simultaneously-alive enemies (protects framerate). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 MaxAliveEnemies = 30;

	/** Enemies spawned per timer tick (grows each wave, up to MaxEnemiesPerSpawn). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 EnemiesPerSpawn = 1;

	/** How much EnemiesPerSpawn grows each wave. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 EnemiesPerSpawnStep = 1;

	/** Ceiling for EnemiesPerSpawn. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 MaxEnemiesPerSpawn = 5;

	/** If true, the spawn ring is centered on the player; otherwise on this spawner. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	bool bSpawnAroundPlayer = true;

	/** Radius of the spawn ring (cm). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float SpawnRadius = 2000.f;

	/** HARD "never spawn outside the map" cap: reject any candidate farther than this (2D) from the
	 *  PLACED spawner, whatever the navmesh says. The cooked-build runtime navmesh can extend past the
	 *  playable floor; this bounds spawns to the arena. Set it to your arena's radius (from the spawner). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float MaxSpawnDistance = 2000.f;

	/** Reject spawn points closer than this to the player (cm). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float MinDistanceFromPlayer = 800.f;

	/** Max placement attempts per spawn before giving up this tick. */
	UPROPERTY(EditAnywhere, Category="Spawning")
	int32 MaxPlacementTries = 10;

	/** Spawn this far above the projected floor point so it settles cleanly (cm). */
	UPROPERTY(EditAnywhere, Category="Spawning")
	float SpawnZOffset = 50.f;

	/** Current wave number (1-based). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	int32 WaveNumber = 0;

	/** Current alive enemy count. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	int32 AliveEnemies = 0;

	/** Blueprint hook for wave UI / stings. */
	UFUNCTION(BlueprintImplementableEvent, Category="Spawning")
	void OnWaveChanged(int32 NewWaveNumber);

private:

	/** Live spawn interval, shrinks each wave toward MinSpawnInterval. */
	float CurrentSpawnInterval = 2.0f;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle WaveTimerHandle;

	/** Spawn-timer callback: spawns up to EnemiesPerSpawn enemies, respecting the cap. */
	void SpawnTick();

	/** Wave-timer callback: escalates difficulty and re-arms the spawn timer. */
	void AdvanceWave();

	/** Attempts one placement + spawn. No-op if capped or no valid point is found. */
	void TrySpawnOne();

	/** Finds a reachable, far-enough spawn point on the NavMesh. Returns false if none. */
	bool FindSpawnPoint(FVector& OutPoint) const;

	/** Rolls a weighted enemy class from EnemyTypes; falls back to EnemyClass. Null if nothing set. */
	TSubclassOf<ACharacter> PickEnemyClass() const;

	/** Bound to each spawned enemy's OnDestroyed; decrements the alive count. */
	UFUNCTION()
	void HandleEnemyDestroyed(AActor* DestroyedActor);
};
