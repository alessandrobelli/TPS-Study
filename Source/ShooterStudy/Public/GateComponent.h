// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GateComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERSTUDY_API UGateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UGateComponent();
    void DoOnce();

    // Gate functions
    UFUNCTION(BlueprintCallable, Category = "Gate")
    void Enter();

    UFUNCTION(BlueprintCallable, Category = "Gate")
    void Open();

    UFUNCTION(BlueprintCallable, Category = "Gate")
    void Close();

    UFUNCTION(BlueprintCallable, Category = "Gate")
    void Toggle();

    bool IsGateOpen() const;
    


    // Flag for whether gate starts closed
    UPROPERTY(EditAnywhere, Category = "Gate")
    bool bStartClosed = true;


    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGateEnteredSignature);

    // Event called when something successfully passes through the gate
    UPROPERTY(BlueprintAssignable, Category = "Gate")
    FOnGateEnteredSignature OnGateEntered;

private:
    // Whether the gate is currently open or closed
    UPROPERTY()
    bool bGateOpen;

    // For tracking if this is the first assignment
    UPROPERTY()
    bool bLocalBoolean = true;
    
    bool bHasBeenInitialized = false;
    bool bIsClosed = true;

    bool bDoOnce = true;
    
};