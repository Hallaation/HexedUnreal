#include "BaseWeapon.h"
#include "Helpers/DebugHelper.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
	TimeLastAttacked = FDateTime::UtcNow();
	DebugHelper::Log("BaseWeapon::BeginPlay()");
}

void ABaseWeapon::DoAttack()
{
	//no longer in cd
	if (not IsInCooldown())
	{
		DebugHelper::Log("BaseWeapon::DoAttack()");
		//reset the cooldown timer
		TimeLastAttacked = FDateTime::UtcNow();
	}
}
bool ABaseWeapon::IsInCooldown()
{
	FTimespan cooldown(0,0,AttackCooldown);
	return (TimeLastAttacked + cooldown) >= FDateTime::UtcNow();
}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}