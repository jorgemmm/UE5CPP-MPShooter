// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"
//#include "Blaster/HUD/OverHeadWidget.h"

#include "Components/TextBlock.h"

//Engine
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/EngineTypes.h"
#include "Engine/Level.h"
#include "Engine/World.h"

void UOverHeadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}


}




void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if (!InPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn is nullptr not posible get role"))
		return;
	}
	ENetRole LocalRole = InPawn->GetLocalRole();
	//ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	switch (LocalRole) //(RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}
	FString RemoteorLocalRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	
	TextComplexToDisplay = RemoteorLocalRoleString;
	SetDisplayText(RemoteorLocalRoleString);

	ShowPlayerName();
}

void UOverHeadWidget::ShowPlayerName()
{
	FString PlayerName = FString::Printf(TEXT("No name Founded"));
	if( GetOwningPlayerState() )
		 PlayerName = GetOwningPlayerState()->GetPlayerName();
	//else 
		//FString PlayerName = GetOwningPlayerPawn()->GetName();
	
	//SetDisplayText(PlayerName); return;

	//TextComplexToDisplay.Append( FString::Printf(TEXT("%s has "), *PlayerName) ); 
	PlayerName = PlayerName.Append(TEXT(" has "));
	TextComplexToDisplay = PlayerName.Append( TextComplexToDisplay );
	
	SetDisplayText(TextComplexToDisplay);

}


void UOverHeadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}


