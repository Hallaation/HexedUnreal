#pragma once
#include "BaseWeapon.h"

#include "Handgun.generated.h"

UCLASS()
class AHandgun : public ABaseWeapon
{
	GENERATED_BODY()
	AHandgun() {  }
	virtual ~AHandgun() override = default;	
protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
};
