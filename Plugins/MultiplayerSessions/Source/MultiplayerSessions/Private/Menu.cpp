// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "GameFramework/PlayerController.h"

//Button
#include "Components/Button.h"

//Engine
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

//Plugin
#include "MultiplayerGISubsystem.h"

#include "OnlineSubsystem.h"






void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;

	AddToViewport();

	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);

		}

		
		//If you wana handle menu from GIsubsystem
		//Coment this this and setup in UMultiplayerGISubsystem::SetupSession
		UGameInstance* GameInstance = GetGameInstance();
		if(GameInstance)
		{
			MultiplayerGISubsystem = GameInstance->GetSubsystem<UMultiplayerGISubsystem>();



			SetUMenuInterface(GameInstance->GetSubsystem<UMultiplayerGISubsystem>());
		    	
			if (MenuInterface)
			{
				MenuInterface->SetupSession(NumberOfPublicConnections, MatchType, PathToLobby);
			}
		}

		
		return;
		//If you waana do it with delegate
		if(MultiplayerGISubsystem)
		{
		 
			MultiplayerGISubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this,&ThisClass::OnCreateSession);					
			

			MultiplayerGISubsystem->MultiplayerOnFindSessionsComplete.AddDynamic(this, &ThisClass::OnFindSessions);			
			
			MultiplayerGISubsystem->MultiplayerOnJoinSessionComplete.AddDynamic(this, &ThisClass::OnJoinSession);
			
			//MultiplayerGISubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
			//MultiplayerGISubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

			

			//Use this delegate to handle aditional features
			MultiplayerGISubsystem->MultiplayerOnFindSessionsResultDelegate.AddUObject(this, &ThisClass::OnFindSessionsResults);
		    
			MultiplayerGISubsystem->MultiplayerOnJoinSessionResultDelegate.AddUObject(this, &ThisClass::OnJoinSessionResults);
		
		}
		
	}
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::SetUMenuInterface(IMenuInterface* DIMenuInterface)
{
	MenuInterface = DIMenuInterface;
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	UE_LOG(LogTemp, Warning, TEXT("Host button clickled"));

	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Host button clickled"))
		);
	}

	if(MenuInterface!=nullptr)
	{
		MenuInterface->Host();
	}
	
	//Code below no need any more if we´ll use D.I.
	
	return;
	
	//If you waana do it with delegate
	if (MultiplayerGISubsystem)
	{
				
		MultiplayerGISubsystem->CreateGameSession(NumPublicConnections, MatchType, PathToLobby);
		//No tengo forma de obtener callback
		//He de crear un delegado propio en MultiplayerGISubsystem
		//para disparar hacia menu.h/cpp un callback indicando que se ha creado la sessión
	}
}


void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	UE_LOG(LogTemp, Warning, TEXT("Join button clicled"));
	
	if (MenuInterface != nullptr)
	{
		MenuInterface->Join();
	}

	//Code below no need any more if we´ll use D.I.
	return;
	//If you waana do it with delegate
	if (MultiplayerGISubsystem)
	{
		
		MultiplayerGISubsystem->FindGameSessions(NumPublicConnections);

	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("Level widget is coming to the end"))
		);
	}
	//When go to new level
	//Remove from parent and close
	//then Remove from level
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}


//Use this methods tho shows info at display
//or disable info

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	//Callback when creste session is complete
	if (bWasSuccessful)
	{
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Session Created Succefully"))
			);
		}
		UE_LOG(LogTemp, Warning, TEXT("Session Created Succefully"));*/

		//MenuInterface->Start();

	}
	else
	{
		HostButton->SetIsEnabled(true);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to create session"))
			);
		}
		UE_LOG(LogTemp, Error, TEXT("Failed to create session"));

	}

}

void UMenu::OnFindSessionsResults(const TArray<class FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (!bWasSuccessful || SessionResults.Num()==0)
	{
		JoinButton->SetIsEnabled(false);
	}
	//Use this function to show a list of active sessions
	//coment it if you don´t use it

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Find sessions Succefully  in another delegate"))
		);
	}
}
void UMenu::OnFindSessions(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Find sessions Succefully"))
			);
		}
		UE_LOG(LogTemp, Warning, TEXT("Find sessions Succefully"));
		
		//MultiplayerGISubsystem->SessionSearch //efectos de visualización
		//por si quelremos hacer lista de sessiones activas.
		/*if (MultiplayerGISubsystem->SessionSearch->SearchResults.Num() == 0) 
		{
			JoinButton->SetIsEnabled(false);
		}  */
	}
	else
	{
		JoinButton->SetIsEnabled(false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to Find sessions"))
			);
		}
		UE_LOG(LogTemp, Error, TEXT("Failed to Find sessions"));

	}

}

//void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
void UMenu::OnJoinSession(bool bWasSuccessful)
{

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Join sessions Succefully"))
			);
		}
		UE_LOG(LogTemp, Warning, TEXT("Join sessions Succefully"));

		//MenuInterface->Start();
	}
	else
	{
		JoinButton->SetIsEnabled(false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to Join sessions"))
			);
		}
		UE_LOG(LogTemp, Error, TEXT("Failed to Join sessions"));

	}
	//MultiplayerGISubsystem->GetJoinResults();
}

void UMenu::OnJoinSessionResults(EOnJoinSessionCompleteResult::Type Result)
{
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Join sessions Succefully in another delegate to take results: %s"), *LexToString(Result))
		);
	}
}




//Method below no need any more if we´ll use D.I.


void UMenu::OnDestroySession(bool bWasSuccessful)
{

}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}
