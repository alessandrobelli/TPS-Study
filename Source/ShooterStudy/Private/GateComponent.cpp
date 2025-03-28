// Fill out your copyright notice in the Description page of Project Settings.


#include "GateComponent.h"

UGateComponent::UGateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGateComponent::DoOnce()
{
    if (bDoOnce)
    {
        bDoOnce = false;
        bLocalBoolean = !bStartClosed;
    }
}

void UGateComponent::Enter()
{
    DoOnce();
    if (bLocalBoolean)
    {
            // call the callback function
           OnGateEntered.Broadcast();
    }
}

void UGateComponent::Open()
{
    DoOnce();
    bLocalBoolean = true;

}

void UGateComponent::Close()
{
    DoOnce();
    bLocalBoolean = false;
}

void UGateComponent::Toggle()
{
    DoOnce(); 
    bLocalBoolean = !bLocalBoolean;
}

auto UGateComponent::IsGateOpen() const -> bool
{
    return bLocalBoolean;
}