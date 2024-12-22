// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	
	return true;
}

void UMenu::NativeDestruct()
{
	MenuTeardown();
	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,
				15.f,
				FColor::Green,
				FString("Create Session Success"));
		}

		UWorld* World = GetWorld();
		if(World)
		{
			World->ServerTravel("/Game/ThirdPerson/Maps/Lobby?listen");
		}
		
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,
				15.f,
				FColor::Red,
				FString("Create Session Failed"));
		}
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	
	if(bWasSuccessful)
	{
		for( auto Result : SearchResults)
		{
			FString Id = Result.GetSessionIdStr();
			FString User = Result.Session.OwningUserName;
			FString SettingsValue;
			Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
			if(GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User));
			}
			if(SettingsValue == MatchType)
			{
				if(GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("Joining Match Type: %s"), *MatchType));
				}
				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
		}
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if(Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if(PlayerController)
			{

				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
		
	}
}

void UMenu::JoinButtonClicked()
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString("JoinButton Clicked"));
	}
	
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTeardown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
		
	}
}
