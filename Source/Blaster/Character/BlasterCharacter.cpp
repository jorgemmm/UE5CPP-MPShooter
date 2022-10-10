// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
//#include "Blaster/Character/BlasterCharacter.h"

//Project
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"

//Engine
#include "Engine/Engine.h"

//Components
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"

//Utils
#include "Kismet/KismetMathLibrary.h"

#include "Net/UnrealNetwork.h"



// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
	
	PrimaryActorTick.bCanEverTick = true;

	
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	SetInAlert(false);
	//SetOrientToMovement(true);	
	/*GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;*/

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;



	bRolling = false;

	MaxHealth = 100;


	
}




// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);

	//=====================
	//!!!!!!Set to false in Constructor PrimaryActorTick.bCanEverTick = false; !!!!!!!!!!!!!!
	//if you don�t use tick mehtod

	return; //comment for debug Set to true PrimaryActorTick.bCanEverTick

	if (!Combat)
	{
		//if (Combat->bAiming)
		return;
	}
	const float Speed = GetCharacterMovement()->Velocity.Length();
	
	//Clients
	if (IsLocallyControlled())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("ownclient tb listenserver: is aiming?: %s to speed: %f "), Combat->bAiming ? TEXT("true") : TEXT("false"), Speed)
			);
		}
	}

	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT(" otros jugadores %s: is aiming?: %s , to speed: %f"), *GetFName().ToString(), Combat->bAiming ? TEXT("true") : TEXT("false"), Speed)
			);
		}
	}



	//server
	if (GetLocalRole() == ROLE_Authority)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Server %s: is aiming?: %s , to speed: %f"), *GetFName().ToString(), Combat->bAiming ? TEXT("true") : TEXT("false"), Speed)
			);
		}
	}


}





// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ABlasterCharacter::RollButtonPressed);
	PlayerInputComponent->BindAction("Roll", IE_Released, this, &ABlasterCharacter::RollButtonReleased);

	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ABlasterCharacter::RunButtonPressed);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ABlasterCharacter::RunButtonReleased);

	
}


void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}


void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		if (Combat->bAiming) return; //To avoid server RPC 
		//Combat->bAiming =true;
		Combat->SetAiming(true);

		SetInAlert(true);
		//SetOrientToMovement(false);

	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		//Combat->bAiming = false;
		Combat->SetAiming(false);
		SetInAlert(false);
		//SetOrientToMovement(true);
		
		
	}
}



void ABlasterCharacter::MoveForward(float Value)
{
	
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::CrouchButtonPressed()
{
	
	if (bIsCrouched)
	{
		UnCrouch();
		SetInAlert(false);
	}
	else
	{

		Crouch();
		SetInAlert(true);
	}
}


void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{

		if ( HasAuthority() )
		{
			//Friend class can acces to private member.
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}

	}
}

//Fire mechanics
//void ABlasterCharacter::Fire()
//{
//	SetOrientToMovement(false);
//	
//	Server_RequestFire();
//}
//
//void  ABlasterCharacter::Server_RequestFire_Implementation()
//{
//	HandeleFire();
//}
//
//void  ABlasterCharacter::HandeleFire()
//{
//   
//	//
//
//}
//Roll mechanic

void ABlasterCharacter::RollButtonPressed()
{
	
	SetRolling(true);
	//bRolling ? false : true;
	//Poner Timer y hasta que no pasen 3 segundos siempre reponder con bIsRolling = false;
	//Poner metodo para que evitar/ignorar todo line trace en esos segundos
}

void ABlasterCharacter::RollButtonReleased()
{
	SetRolling(false);	

}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if( Speed == 0.f && !bIsInAir ) //Standing still, not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		bUseControllerRotationYaw = false;

	}

	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	//UE_LOG(LogTemp, Error, TEXT("AO_Pitch: %f "), AO_Pitch);

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
		UE_LOG(LogTemp, Warning, TEXT("AO_Pitch with clamp: %f "), AO_Pitch);
	}

}

void ABlasterCharacter::RunButtonPressed()
{
	

	SetRunning(true);
	WalkSpeed = RunSpeed;
	if (Combat)
	{
		Combat->SetAiming(false);  //Not in combat if run
		SetInAlert(false); //To propagate WalkSpeeed
	}

}

void ABlasterCharacter::RunButtonReleased()
{
	SetRunning(false);
	WalkSpeed = DefaulWalkSpeed;
	SetInAlert(false); //To propagate WalkSpeeed
}



//Si bRolling cambia se notificar� este met�do en todos los clientes
void ABlasterCharacter::OnRep_bRolling()//(bool bIsRolling)
{
	UE_LOG(LogTemp, Warning, TEXT("Repnotify work out"));
	//Use this function to avoid damage 
	//for example To request server change Collision, ignore projectile. etc.

	//No est� funcionando no se dispara. =???

	// and show 

	if (GetLocalRole() == ROLE_AutonomousProxy) {
		FString rollingMessage = FString::Printf(TEXT("Roll button press %s"), (bRolling ? TEXT("true") : TEXT("true")));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, rollingMessage);

		//bRolling = bIsRolling;
		UE_LOG(LogTemp, Warning, TEXT("Roll button press %s"), (bRolling ? TEXT("true") : TEXT("true")));
	}

	//Client-specific functionality
	if (IsLocallyControlled())
	{   
		FString rollingMessage = FString::Printf(TEXT("Roll button press %s"), (bRolling ? TEXT("true") : TEXT("true")));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, rollingMessage);

		UE_LOG(LogTemp, Warning, TEXT(" Client:   Roll button press %s"), (bRolling ? TEXT("true") : TEXT("true")));
		
		//Health for debug
		
		/*FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}*/
		
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	 {

		FString serverRollingMessage = FString::Printf(TEXT(" %s now has Roll button press %s"), *GetFName().ToString(), (bRolling ? TEXT("true") : TEXT("true")));
		if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, serverRollingMessage);
		
		UE_LOG(LogTemp, Warning, TEXT(" %s now has Roll button press %s"), *GetFName().ToString(), (bRolling ? TEXT("true") : TEXT("true")));
		
		/*FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);*/


	}

	//Cualquier funcionalidad que deba ocurrir en todas las m�quinas. Clientes y servidor. 

	
}

//Petici�n local
void ABlasterCharacter::SetRolling(bool bIsRolling)
{
	bRolling = bIsRolling;
	Server_RequestSetRolling(bIsRolling);
}


//RPC to server:  Petici�n a servidor
void ABlasterCharacter::Server_RequestSetRolling_Implementation(bool bIsRolling)
{
	bRolling = bIsRolling;
}


//Petici�n local
void ABlasterCharacter::SetRunning(bool bIsRunning)
{
	bRunning = bIsRunning;
	Server_RequestSetRunning(bIsRunning);
}


//RPC to server:  Petici�n a servidor
void ABlasterCharacter::Server_RequestSetRunning_Implementation(bool bIsRunning)
{
	bRunning = bIsRunning;
}

//Equip mechanic
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}



void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
//void ABlasterCharacter::OnRep_OverlappingWeapon()
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}

}



void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{   
	if (OverlappingWeapon) OverlappingWeapon->ShowPickupWidget(false);		
	
	OverlappingWeapon = Weapon;
	
	if (IsLocallyControlled()) // tanto en server como en 
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
		
	}
}

void ABlasterCharacter::SetInAlert(bool inAlert)//(bool OrientRotation)
{	
	GetCharacterMovement()->bOrientRotationToMovement = !inAlert;
	
	bUseControllerRotationYaw = inAlert;
	
	GetCharacterMovement()->MaxWalkSpeed = inAlert? AimSpeed : WalkSpeed;
	 
}



bool ABlasterCharacter::IsWeaponEquipped() const
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming() const 
{
	return (Combat && Combat->bAiming);
}

bool ABlasterCharacter::IsRolling() const
{
	return bRolling;
}


bool ABlasterCharacter::IsRunning() const
{
	return bRunning;
}


AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, bRolling);
	DOREPLIFETIME(ABlasterCharacter, bRunning);

}




