#include "HwPawn.h"
#include "Hw_09/Hw_09.h"
AHwPawn::AHwPawn()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHwPawn::BeginPlay()
{
	Super::BeginPlay();
	FString NetRoleString = ChatXFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"), *ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
	
}

void AHwPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	FString NetRoleString = ChatXFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"), *ChatXFunctionLibrary::GetNetModeString(this), *NetRoleString);
	ChatXFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}