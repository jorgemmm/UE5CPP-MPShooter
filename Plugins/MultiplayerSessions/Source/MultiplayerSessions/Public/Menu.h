// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

//Interfaces
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MenuInterface.h"

#include "Menu.generated.h"


/**
 Víctor J. Plasín Vicagent
 */

//class UButton;
class UMultiplayerGISubsystem;
class FOnlineSessionSearchResult;
 

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));


protected:
	
	virtual bool Initialize() override;

	

	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	
	IMenuInterface* MenuInterface;

	//If you waana do it with delegate
	UMultiplayerGISubsystem* MultiplayerGISubsystem; //Deprected you must delete

private:

	
	UPROPERTY(meta = (BindWidget))
		class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
		class UButton* JoinButton;
	
	UFUNCTION()
		void HostButtonClicked();

	UFUNCTION()
		void JoinButtonClicked();

	void MenuTearDown();

	
	//Valores default 
	//
	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };


public:

	

	void SetUMenuInterface(IMenuInterface* DIMenuInterface);	

	

	//Code below no need any more if we´ll use D.I.
	//This property no need any more if we´ll use D.I.
	// The subsystem designed to handle all online session functionality

	



protected:

	//This method are callback by D.I. 

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION() //Requiere epic macro por ser delegado dinámico
		void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
		void OnFindSessions(bool bWasSuccessful);
	//UFUNCTION()
	void OnFindSessionsResults(const TArray<class FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	UFUNCTION()
		void OnJoinSession(bool bWasSuccessful);
	
	void OnJoinSessionResults(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
		void OnStartSession(bool bWasSuccessful);
};
