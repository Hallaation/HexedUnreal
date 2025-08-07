#include "H:\proj\HexedUnreal\Intermediate\Build\Win64\x64\HexedUnrealEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.ValExpApi.Cpp20.h"
#include "Handgun.h"
#include "Helpers/DebugHelper.h"

void AHandgun::BeginPlay()
{
	Super::BeginPlay();
}

void AHandgun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DoAttack();
}
