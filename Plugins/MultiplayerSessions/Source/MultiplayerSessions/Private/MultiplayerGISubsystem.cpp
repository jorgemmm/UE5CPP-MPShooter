// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerGISubsystem.h"


//To handle menu from here
#include "Menu.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

//Components
#include "GameFramework/PlayerController.h"


//Engine
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"




//Networking MP online
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "Engine/EngineBaseTypes.h"




//const static FName MY_SESSION_NAME = TEXT("MY Session Name");

UMultiplayerGISubsystem::UMultiplayerGISubsystem()
{
	IOnlineSubsystem* PluginOnlineSubsystem = IOnlineSubsystem::Get();

	if (PluginOnlineSubsystem)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Found subsystem %s"), *PluginOnlineSubsystem->GetSubsystemName().ToString())
			);
		}
		OnlineSessionInterface = PluginOnlineSubsystem->GetSessionInterface();

		//otra forma de crear el callback al crear la session
		if (OnlineSessionInterface.IsValid())
		{

			//Mejor por D.I.

			//OlineSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &AMenuSystemCharacter::OnCreateSessionComplete);
			OnlineSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateSessionComplete);

			OnlineSessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);

			//OnlineSessionInterface->OnFindFriendSessionCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsResultComplete);

			OnlineSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);

			OnlineSessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &ThisClass::OnStartSessionComplete);

			OnlineSessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnDestroySessionComplete);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OlineSessionInterface  Character::Constructor in not valid or nullptr"));
		}

		
		return;
		
		//World'/Game/Maps/GameStarterMap.GameStarterMap' Y:/Unreal Projects/UE5/Udemy/CppShooter/Blaster/Content/Maps/BlasterMap.umap
		//WidgetBlueprint'/MultiplayerSessions/WBP_Menu.WBP_Menu' Y:/Unreal Projects/UE5/Udemy/CppShooter/Blaster/Plugins/MultiplayerSessions/Content/WBP_Menu.uasset
		ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Plugins/MultiplayerSessions/WBP_Menu"));
		if (!ensure(MenuBPClass.Class != nullptr)) return;
		MenuClass = MenuBPClass.Class;
		

	
	}
}

void UMultiplayerGISubsystem::Host()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Cyan,
			FString::Printf(TEXT("Host  by Interface"))
		);
	}
	
	CreateGameSession(4, FString{ TEXT("FreeForAll") }, FString("/Game/Maps/Lobby?listen"));
}

void UMultiplayerGISubsystem::SetupSession(int32 NumPublicConnections, FString MatchType, FString PathToLobby)
{
	LastNumPublicConnections = NumPublicConnections;
	LastMatchType = MatchType;
	LastPathLobby = PathToLobby;
	
	
	

	UWorld* world = GetWorld();

	if (!ensure(MenuClass != nullptr)) return;
	MainMenu = CreateWidget<UMenu>(world, MenuClass);

	
	
	return;

	//To handle menu from here
	// 
	//MainMenu->MenuSetup();

	//MainMenu->SetUMenuInterface(this);
}



void UMultiplayerGISubsystem::Join()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Cyan,
			FString::Printf(TEXT("Join  by Interface"))
		);
	}
	FindGameSessions(4);
	//FindGameSessions(NumPublicConnections)
}

void UMultiplayerGISubsystem::Start()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("start session"))
		);
	}
	//OnlineSessionInterface->StartSession(NAME_GameSession);
}

void UMultiplayerGISubsystem::CreateGameSession(int32 NumPublicConnections, FString MatchType, FString PathToLobby)
{
	if (!OnlineSessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OlineSessionInterface  Character::CreateSession in not valid or nullptr"));
		return;
	}

	LastPathLobby = PathToLobby;

	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
			
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
	
		DestroySession();

	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Host to Match type: %s, on Lobby: %s"), *MatchType, *LastPathLobby)
			);
		}

		CreateSession(NumPublicConnections, MatchType);
		
	}


	/*auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) 
	{

		

		DestroySession();
	}*/
	




	
	



	
}


void UMultiplayerGISubsystem::FindGameSessions(int32 MaxSearchResults)
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);


	const ULocalPlayer* LocalPlayer = this->GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}


void UMultiplayerGISubsystem::JoinGameSession(bool WasSucceful)
{
	
	if (!OnlineSessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineSessionInterface in Character::JoinGameSession in not valid or nullptr"));

		return;
	}

	FindGameSessions(100);
	
}



void UMultiplayerGISubsystem::DestroySession()
{
	if(OnlineSessionInterface)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Destroy Session..."))
			);
		}
		OnlineSessionInterface->DestroySession(NAME_GameSession);//(NAME_GameSession);
	
	}


}

void UMultiplayerGISubsystem::StartSession()
{
	OnlineSessionInterface->StartSession(NAME_GameSession);
}

void UMultiplayerGISubsystem::OnCreateSessionComplete(FName SessionName, bool WasSucceful)
{
	//Callback from delegate
	// when Create Session

	//if (OnlineSessionInterface.IsValid())
	//{ 
	//  OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	//	OnlineSessionInterface->ClearOnCreateSessionCompleteDelegates(this); 
	//}

	
	//MultiplayerOnCreateSessionComplete.Broadcast(WasSucceful);
	
	if (WasSucceful)
	{
		

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Create session succesfully: %s"), *SessionName.ToString())
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{

			//World'/Game/Maps/Lobby.Lobby'
			//World->ServerTravel(FString("/Game/Maps/Lobby?listen"));
			World->ServerTravel(FString(LastPathLobby));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OlineSessionInterface  Character::OnCreateSessionComplete Failed to create session"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to create session"))
			);
		}

	


	}
}

void UMultiplayerGISubsystem::OnFindSessionsComplete(bool WasSucceful)
{
	
	if (!OnlineSessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OlineSessionInterface  AMenuSystemCharacter::OnFindSessionsComplete in not valid or nullptr"));
		return;
	}

	if (WasSucceful && SessionSearch.IsValid())
	{
		
		
		if ( SessionSearch->SearchResults.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Find session complete succes  but SessionSearch->SearchResults is empty"));

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString::Printf(TEXT("Find session complete but SessionSearch->SearchResults is empty"))
				);
			}

			

			return;

		}

		/*if (GEngine) {
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("find session"))
			);
		}*/
		for(const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		//for (auto Result : SessionSearch->SearchResults)
		{
			

			const FString& Id = Result.GetSessionIdStr();
			const FString& User = Result.Session.OwningUserName;
			FString MatchType;

			Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
			//UE_LOG(LogTemp, Warning, TEXT("found session name: %s, and user: %s"), *Id, *User);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Yellow,
					FString::Printf(TEXT("found session name: %s, and user: %s"),
						*Id,//*Result.GetSessionIdStr(),
						*User)//*Result.Session.OwningUserName)
				);
			}
			if (MatchType == FString("FreeForAll"))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Yellow,
						FString::Printf(TEXT("Joining to match: %s"), *MatchType)
					);
				}

				//OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinGameSession);
				const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
				if (LocalPlayer) {

					if (!OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result))
					{
						UE_LOG(LogTemp, Warning, TEXT("Find session complete"));
					}
				}


				//OnlineSessionInterface->JoinSession(0, NAME_GameSession, Result);
			}
		}

		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Error Searching sessions..."))
		);

		UE_LOG(LogTemp, Warning, TEXT("Error Searching sessions..."));

	}
}

void UMultiplayerGISubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("OlineSessionInterface  AMenuSystemCharacter::OnJoinSessionComplete in not valid or nullptr"));
		return;
	}
	FString Adress;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Adress))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf(TEXT("Connect string: %s"), *Adress)
			);
		}

		UWorld* World = GetWorld();
		if (World)
		{


			//APlayerController* LocalPlayer = World->GetGameInstance()->GetFirstLocalPlayerController();

			APlayerController* LocalPlayer = GetGameInstance()->GetFirstLocalPlayerController();
			if (LocalPlayer)
			{
				LocalPlayer->ClientTravel(Adress, ETravelType::TRAVEL_Absolute);

			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to client travel"));
		}

	}
}

void UMultiplayerGISubsystem::OnDestroySessionComplete(FName SessionName, bool WasSucceful)
{
	if (WasSucceful)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString::Printf(TEXT("Destroying session: %s"), *SessionName.ToString())
		);

		UE_LOG(LogTemp, Warning, TEXT("OlineSessionInterface  Character::OnDestroySessionComplete Destroying session: %s"), *SessionName.ToString());

		//Don´t use this callback to create session
		if (bCreateSessionOnDestroy)
		{
			CreateSession(LastNumPublicConnections, LastMatchType);
			bCreateSessionOnDestroy = false;
		}
	}
}

void UMultiplayerGISubsystem::OnStartSessionComplete(FName SessionName, bool WasSucceful)
{

}

void UMultiplayerGISubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	TSharedPtr< FOnlineSessionSettings > SessionSettingsUE5 = MakeShareable(new FOnlineSessionSettings());
	
	//SessionSettingsUE5->bIsLANMatch = true; //con NULL
	SessionSettingsUE5->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	
	SessionSettingsUE5->NumPublicConnections = NumPublicConnections;

	SessionSettingsUE5->bAllowJoinInProgress = true;
	SessionSettingsUE5->bAllowJoinViaPresence = true;

	SessionSettingsUE5->bShouldAdvertise = true;

	SessionSettingsUE5->bUsesPresence = true;

	SessionSettingsUE5->bUseLobbiesIfAvailable = true; //If UE5

	SessionSettingsUE5->BuildUniqueId = 1;


	SessionSettingsUE5->Set(FName("MatchType"), 
		MatchType,//FString("FreeForAll"), 
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);


	const ULocalPlayer* LocalPlayer = this->GetWorld()->GetFirstLocalPlayerFromController();
	if(!OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettingsUE5))
	{
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegates(this);
		//MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
	
}



void UMultiplayerGISubsystem::EndSession()
{

}

////Deprecated

//
//void UMultiplayerGISubsystem::FindSession()
//{
//	//Deprecated
//
//	SessionSearch = MakeShareable(new FOnlineSessionSearch());
//	SessionSearch->bIsLanQuery = true;
//	SessionSearch->MaxSearchResults = 10000;
//
//
//
//	const ULocalPlayer* LocalPlayer = this->GetWorld()->GetFirstLocalPlayerFromController();
//	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
//}
//
