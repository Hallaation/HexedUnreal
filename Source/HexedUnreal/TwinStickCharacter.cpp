// Copyright Epic Games, Inc. All Rights Reserved.
#include "DrawDebugHelpers.h"

#include "TwinStickCharacter.h"

#include <tiffio.h>

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "TwinStickGameMode.h"
#include "TwinStickAoEAttack.h"
#include "Kismet/KismetMathLibrary.h"
#include "TwinStickProjectile.h"
#include "GameFramework/InputDeviceProperties.h"
#include "Helpers/DebugHelper.h"
#include "Net/UnrealNetwork.h"

ATwinStickCharacter::ATwinStickCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the spring arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	SpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	SpringArm->TargetArmLength = 500.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 0.5f;

	// create the camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Camera->SetFieldOfView(75.0f);
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);

	// configure the character movement
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	
}

void ATwinStickCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerController != nullptr)
	{
		PlayerController->SetReplicates(true);
		PlayerController->SetReplicateMovement(true);
	}
	// update the items count
	UpdateItems();
}

void ATwinStickCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// set the player controller reference
	PlayerController = Cast<APlayerController>(GetController());
}

void ATwinStickCharacter::DoPlayerRotation()
{
	//Check for PlayerController
	if (PlayerController == nullptr)
	{
		return;
	}
	
	// are we aiming with the mouse?
	if (bUsingMouse)
	{
		// get the cursor world location
		FHitResult OutHit;
		PlayerController->GetHitResultUnderCursorByChannel(MouseAimTraceChannel, true, OutHit);

		// find the aim rotation 
		const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OutHit.Location);
		// save the aim angle
		AimAngle = AimRot.Yaw;
	}

	const FRotator OldRotation = GetActorRotation();
	//AimAngle either from controller input event, or earlier when using mouse controls.
	const FRotator targetRot = FRotator(OldRotation.Pitch, AimAngle, OldRotation.Roll);
	PlayerController->SetControlRotation(targetRot);
}


void ATwinStickCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DoPlayerRotation();
}

void ATwinStickCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up the enhanced input action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::Move);
		EnhancedInputComponent->BindAction(StickAimAction, ETriggerEvent::Triggered, this,
		                                   &ATwinStickCharacter::StickAim);
		EnhancedInputComponent->BindAction(MouseAimAction, ETriggerEvent::Triggered, this,
		                                   &ATwinStickCharacter::MouseAim);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::Dash);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this,
		                                   &ATwinStickCharacter::Shoot);
		EnhancedInputComponent->BindAction(AoEAction, ETriggerEvent::Triggered, this,
		                                   &ATwinStickCharacter::AoEAttack);
	}
}

void ATwinStickCharacter::Move(const FInputActionValue& Value)
{
	// save the input vector
	FVector2D InputVector = Value.Get<FVector2D>();

	//Use input vector as the rotation when using gamepad
	if (not bUsingMouse && InputVector.SquaredLength() > 0)
	{
		FVector2d RotVector(-InputVector.X, InputVector.Y);
		DoAim(RotVector.X, RotVector.Y);
		DrawDebugAimLine(FColor::Red, RotVector);
	}
	// route the input
	DoMove(InputVector.X, InputVector.Y);
}

void ATwinStickCharacter::StickAim(const FInputActionValue& Value)
{
	// get the input vector
	FVector2D InputVector = Value.Get<FVector2D>();
	//only accept input if theres actual input

	if (InputVector.SquaredLength() > 0)
	{
		// route the input
		DoAim(InputVector.X, InputVector.Y);
		DrawDebugAimLine(FColor::Blue, InputVector);
	}
}

void ATwinStickCharacter::MouseAim(const FInputActionValue& Value)
{
	// raise the mouse controls flag
	bUsingMouse = true;

	// show the mouse cursor
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(true);
	}
}

void ATwinStickCharacter::Dash(const FInputActionValue& Value)
{
	// route the input
	DoDash();
}

void ATwinStickCharacter::Shoot(const FInputActionValue& Value)
{
	// route the input
	DoShoot();
}

void ATwinStickCharacter::AoEAttack(const FInputActionValue& Value)
{
	// route the input
	DoAoEAttack();
}

void ATwinStickCharacter::DoMove(float AxisX, float AxisY)
{
	// save the input
	LastMoveInput.X = AxisX;
	LastMoveInput.Y = AxisY;
	
	//Forward/Right vector based on world vectors, not local control vectors, since the rotation is set to the control
	AddMovementInput(FVector::ForwardVector, AxisX);
	AddMovementInput(FVector::RightVector, AxisY);
}

void ATwinStickCharacter::DoAim(float AxisX, float AxisY)
{
	// calculate the aim angle from the inputs
	AimAngle = FMath::RadiansToDegrees(FMath::Atan2(AxisY, -AxisX));

	//DebugHelper::Log("ATwinStickCharacter::DoAim");
	// lower the mouse controls flag
	bUsingMouse = false;

	// hide the mouse cursor
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
	}
}

void ATwinStickCharacter::DoDash()
{
	// calculate the launch impulse vector based on the last move input
	FVector LaunchDir = FVector::ZeroVector;

	LaunchDir.X = FMath::Clamp(LastMoveInput.X, -1.0f, 1.0f);
	LaunchDir.Y = FMath::Clamp(LastMoveInput.Y, -1.0f, 1.0f);

	// launch the character in the chosen direction
	LaunchCharacter(LaunchDir * DashImpulse, true, true);
}

void ATwinStickCharacter::DoShoot()
{
	// get the actor transform
	FTransform ProjectileTransform = GetActorTransform();

	// apply the projectile spawn offset
	FVector ProjectileLocation = ProjectileTransform.GetLocation() + ProjectileTransform.GetRotation().RotateVector(
		FVector::ForwardVector * ProjectileOffset);
	ProjectileTransform.SetLocation(ProjectileLocation);

	ATwinStickProjectile* Projectile = GetWorld()->SpawnActor<ATwinStickProjectile>(
		ProjectileClass, ProjectileTransform);
}

void ATwinStickCharacter::DoAoEAttack()
{
	// do we have enough items to do an AoE attack?
	if (Items > 0)
	{
		// get the game time
		const float GameTime = GetWorld()->GetTimeSeconds();

		// are we off AoE cooldown?
		if (GameTime - LastAoETime > AoECooldownTime)
		{
			// save the new AoE time
			LastAoETime = GameTime;

			// spawn the AoE
			ATwinStickAoEAttack* AoE = GetWorld()->SpawnActor<ATwinStickAoEAttack>(
				AoEAttackClass, GetActorTransform());

			// decrease the number of items
			--Items;

			// update the items count
			UpdateItems();
		}
	}
}

void ATwinStickCharacter::HandleDamage(float Damage, const FVector& DamageDirection)
{
	// calculate the knockback vector
	FVector LaunchVector = DamageDirection;
	LaunchVector.Z = 0.0f;

	DebugHelper::Log("HandleDamage");
	// apply knockback to the character
	LaunchCharacter(LaunchVector * KnockbackStrength, true, true);

	// pass control to BP
	BP_Damaged();
}

void ATwinStickCharacter::AddPickup()
{
	// increase the item count
	++Items;

	// update the items counter
	UpdateItems();
}

void ATwinStickCharacter::UpdateItems()
{
	// update the game mode
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ItemUsed(Items);
	}
}

#ifdef UE_BUILD_DEBUG
void ATwinStickCharacter::DrawDebugAimLine(const FColor InColor, const FVector2d InAxis) const
{
	FVector3d actorLoc = GetActorLocation();
	DrawDebugLine(GetWorld(), actorLoc, actorLoc + (FVector(-InAxis.X, InAxis.Y, 0) * 150), InColor, false, 0,
	              5);
}
#endif
