#pragma once
#include "BaseWeapon.generated.h"

UCLASS(abstract)
class ABaseWeapon : public AActor
{
	GENERATED_BODY()

	//ctors/destructors
public:
	ABaseWeapon();
	virtual ~ABaseWeapon() override = default;

protected:
	virtual void BeginPlay() override;

private:
protected:
	virtual bool IsInCooldown();

public:
	virtual void DoAttack();
	
	virtual void Tick(float DeltaTime) override;
	
public:
	int GetLastOwnedPlayerID() const { return LastOwnedPlayerID; }
	void SetLastOwnedPlayerID(const int PlayerID) {  LastOwnedPlayerID = PlayerID; }

	//Variables
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//Time in seconds
	float AttackCooldown = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Damage = 1;

protected:
	//Time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 1.5f;

	FDateTime TimeLastAttacked;
	//Used to keep track of who or what this weapon does
	int LastOwnedPlayerID = -1;
};
