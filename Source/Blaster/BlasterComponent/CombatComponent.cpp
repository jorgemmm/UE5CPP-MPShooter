// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
///#include "Blaster/BlasterComponent/CombatComponent.h"

#include "Blaster/Weapon/Weapon.h"

#include "Blaster/Character/BlasterCharacter.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.

	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}


// Called every frame



void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Local setup Aiming
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	Server_RequestSetAiming(bIsAiming);
}

//Request to server -> bAimig replicated from server to all autonomous proxies ( oww clients) 
void UCombatComponent::Server_RequestSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{    
		// Request to server from a comp.
		//Character movement replicates from server
		//Character->GetCharacterMovement()->MaxWalkSpeed = bAiming? AimWalkSpeed : BaseWalkSpeed;
		Character->SetInAlert(true);
	}
}


void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{		
		//Request to server replicates, every changes in EquippedWeapon
		//Every Character own client set in alert. 		
		
		Character->SetInAlert(true);
		
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	//Character->

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	//Owner inherents an repNotify -> CombatComponent::Owner propagate change to all owner client 
	EquippedWeapon->SetOwner(Character);

	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);

}



