// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/OnlineSessionInterface.h"

#include "MenuInterface.h"

#include "MultiplayerGISubsystem.generated.h"

/**
 Víctor J. Plasín Vicagent 
 */

class FOnlineSessionSearchResult;

 //
 // Delcaring our own custom delegates for the Menu class to bind callbacks to
 //

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnCreateSessionComplete, bool, bWasSuccesful);
//
DECLARE_MULTICAST_DELEGATE_TwoParams(FGIMultiplayerOnFindSessionsResult, const TArray<class FOnlineSessionSearchResult>& SessionResults, bool  bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnJoinSessionResult, EOnJoinSessionCompleteResult::Type Result);
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnFindSessionsComplete, bool,  bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnJoinSessionComplete, bool, bWasSuccessful);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGIMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerGISubsystem : public UGameInstanceSubsystem, public IMenuInterface
{
	GENERATED_BODY()

public:

	UMultiplayerGISubsystem();


	/**	
	  Interfaces methods, implements too in Menu
     
	*/
		UFUNCTION(Exec)
	    void Host() override;

		UFUNCTION(Exec)
		void SetupSession(int32 NumPublicConnections, FString MatchType, FString PathToLobby) override;
		
		UFUNCTION(Exec)
		void Join() override;

		UFUNCTION(Exec)
		void Start() override;
	
		//
	// Our own custom delegates for the Menu class to bind callbacks to
		//
		FGIMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;

		FGIMultiplayerOnFindSessionsResult    MultiplayerOnFindSessionsResultDelegate;	

		FGIMultiplayerOnFindSessionsComplete  MultiplayerOnFindSessionsComplete;
		
		FGIMultiplayerOnJoinSessionComplete   MultiplayerOnJoinSessionComplete;

		FGIMultiplayerOnJoinSessionResult     MultiplayerOnJoinSessionResultDelegate;
		//FGIMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
		//FGIMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

		
		/*UFUNCTION(Exec)
		void Join(const FString& Address) override;*/

	    UFUNCTION(BlueprintCallable)
		void CreateGameSession(int32 NumPublicConnections, FString MatchType, FString PathToLobby);

		UFUNCTION(BlueprintCallable)
		void FindGameSessions(int32 MaxSearchResults);

		//SUFUNCTION(BlueprintCallable)
	    void JoinGameSession(bool WasSucceful);
		//UFUNCTION(BlueprintCallable)
		void DestroySession();
		//UFUNCTION(BlueprintCallable)
		void StartSession();		

		

private:
    
	//To handel menu from here
	TSubclassOf<class UUserWidget> MenuClass;
	class UMenu* MainMenu;

	//TSubclassOf<class UUserWidget> InGameMenuClass;

		
protected:
	/**
	Internal callbacks for the delagates, we´ll add to the onlione Session interface delegate list.
	This don´t nedd to be called outside this class
	*/
	void OnCreateSessionComplete(FName SessionName, bool WasSucceful);
	void OnFindSessionsComplete(bool WasSucceful);	
	

	//No funciona el delagado con tipos complejos solo UClass, Enum, etc,..
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	
	void OnDestroySessionComplete(FName SessionName, bool WasSucceful);
	void OnStartSessionComplete(FName SessionName, bool WasSucceful);

    
	

private:
	
	///pointer to the session interface
	IOnlineSessionPtr OnlineSessionInterface;
	
	

	void CreateSession(int32 NumPublicConnections, FString MatchType);
	

	void EndSession();

	

	

	//Deprecated
	//void FindSession();

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
	FString LastPathLobby;

public:
	
	TSharedPtr< class FOnlineSessionSearch > SessionSearch;

	
};
