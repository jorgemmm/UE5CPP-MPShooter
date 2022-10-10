// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	//It�s disable in constructor
	

	friend class ABlasterCharacter; //For Blaster charater can acces to protected/private member
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:

	virtual void BeginPlay() override;

	//petici�n local
	void SetAiming(bool bIsAiming);
	
	//Solicitud a servidor
	UFUNCTION(Server, Reliable)
	void Server_RequestSetAiming(bool bIsAiming);
	
	UFUNCTION()
		void OnRep_EquippedWeapon();

private:

	ABlasterCharacter* Character;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
		bool bAiming;

	float BaseWalkSpeed = 600.f;
	float  AimWalkSpeed = 450.f;
public:	


	

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
};
