// Fill out your copyright notice in the Description page of Project Settings.

#include "ParticleRush.h"
#include "RushCharacter.h"
#include "RushCharacterMovementComponent.h"
#include "RushCameraComponent.h"
#include "RushCameraArmComponent.h"


ARushCharacter::ARushCharacter(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<URushCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
#pragma region Component Definitions
	// Create Capsule Component
	UCapsuleComponent* capsuleComponent = GetCapsuleComponent();
	capsuleComponent->InitCapsuleSize(34.0f, 48.0f);
	capsuleComponent->OnComponentHit.AddDynamic(this, &ARushCharacter::OnCapsuleCollision);

	// Create a spring arm component
	RushCameraBoom = ObjectInitializer.CreateDefaultSubobject<URushCameraArmComponent>(this, TEXT("RushCameraBoom"));
	RushCameraBoom->AttachTo(capsuleComponent);	
	
	// Create camera component 
	RushCamera = ObjectInitializer.CreateDefaultSubobject<URushCameraComponent>(this, TEXT("RushCamera"));
	RushCamera->AttachTo(RushCameraBoom, USpringArmComponent::SocketName);
	RushCamera->bUsePawnControlRotation = false;

	// Create action sphere component
	RushActionSphere = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("RushActionSphere"));
	RushActionSphere->AttachTo(RootComponent);
	RushActionSphere->OnComponentBeginOverlap.AddDynamic(this, &ARushCharacter::OnRushActionSphereBeginOverlap);
	RushActionSphere->OnComponentEndOverlap.AddDynamic(this, &ARushCharacter::OnRushActionSphereEndOverlap);

	// Create Point Light Component
	RushNavigationLight = ObjectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, TEXT("RushNavigationLight"));
	RushNavigationLight->AttachTo(RootComponent);
#pragma endregion

#pragma region Behavior Parameter Setups
	InitializeBehaviorMovement();

	InitializeBehaviorBounce();

	InitializeBehaviorBoost();

	#pragma region Sharp Turn
	_sharpTurnTarget = FRotator(0.0f, 0.0f, 0.0f);
	#pragma endregion

	#pragma region Hard Stop
	_timeLeftForHardStopToEnd = -1.0f;
	_hardTurnTarget = FRotator(0.0f, 0.0f, 0.0f);
	#pragma endregion
#pragma endregion

#pragma region Rush Action Sphere Timer Management
	ResetRushTimeScale();
#pragma endregion
}


#pragma region Base Class Overrides
void ARushCharacter::BeginPlay()
{
#pragma region OnBegin Behavior Setup
	OnBeginPlayBehaviorMovement();
#pragma endregion

	Super::BeginPlay();
}


void ARushCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RushCameraBoom->SetCamera(RushCamera);
	RushCameraBoom->RequestCameraStageSwitch(0);
}


void ARushCharacter::Tick(float DeltaSeconds)
{
	//ExecuteRushTimeScaleUpdatePerTick(DeltaSeconds);

	ExecuteBoostPerTick(DeltaSeconds);

	ExecuteBouncePerTick(DeltaSeconds);

	ExecuteMeshRotationPerTick(DeltaSeconds);	

	Super::Tick(DeltaSeconds);
}


void ARushCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ARushCharacter::MoveForward);
	InputComponent->BindAxis("TurnRight", this, &ARushCharacter::TurnRight);
	InputComponent->BindAxis("SharpTurn", this, &ARushCharacter::ActivateSharpTurn);
	InputComponent->BindAction("HardStop", IE_Pressed, this, &ARushCharacter::ActivateHardStop);
	InputComponent->BindAction("Boost", IE_Pressed, this, &ARushCharacter::ActivateBoost);
	InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ARushCharacter::SwitchCamera);
}
#pragma endregion


#pragma region Physics Methods and Callbakcs
void ARushCharacter::OnCapsuleCollision(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	BounceAgainstWall(OtherActor, HitResult);
}

void ARushCharacter::OnRushActionSphereBeginOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	#pragma region Is Collision with Wall?

	#pragma endregion
}

void ARushCharacter::OnRushActionSphereEndOverlap(class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
#pragma region Has Collision with Wall Ended?

#pragma endregion
}
#pragma endregion


#pragma region Rush Input
void ARushCharacter::ActivateSharpTurn(float value)
{
	if (Controller != NULL && value != 0.0f && _sharpTurnTarget.IsNearlyZero())
	{
		_sharpTurnTarget = Controller->GetControlRotation() + FRotator(0.0f, value * 90.0f, 0.0f);
	}
}


void ARushCharacter::ActivateHardStop()
{
	_timeLeftForHardStopToEnd = RushData.HardStopDriftDuration;
	_hardTurnTarget = GetController()->GetControlRotation() + FRotator(0.0f, 180.0f, 0.0f);	
}


void ARushCharacter::SwitchCamera()
{
	RushCameraBoom->RequestCameraStageSwitch();
}
#pragma endregion


#pragma region Rush Behaviors
void ARushCharacter::ExecuteSharpTurnPerTick(float deltaSeconds)
{
	if (_sharpTurnTarget == FRotator(0.0f, 0.0f, 0.0f))
		return;

	if (Controller != NULL)
	{
		FRotator currentControllerRotation = Controller->GetControlRotation();
		FRotator difference = _sharpTurnTarget - currentControllerRotation;

		if (difference.IsNearlyZero())
		{
			_sharpTurnTarget = FRotator(0.0f, 0.0f, 0.0f);
		}
		else
		{
			FRotator interpControllerRotation = FMath::RInterpTo(currentControllerRotation, _sharpTurnTarget, deltaSeconds, RushData.SharpTurnStrength);
			Controller->SetControlRotation(interpControllerRotation);
		}
	}
}


void ARushCharacter::ExecuteHardStopPerTick(float deltaSeconds)
{
	if (_hardTurnTarget == FRotator(0.0f, 0.0f, 0.0f))
		return;

	if (Controller != NULL)
	{
		FRotator currentControllerRotation = Controller->GetControlRotation();
		FRotator difference = _hardTurnTarget - currentControllerRotation;

		if (difference.IsNearlyZero())
		{
			_timeLeftForHardStopToEnd -= deltaSeconds;

			if (_timeLeftForHardStopToEnd < 0.0f)
			{
				_timeLeftForBoostToEnd = -1.0f;
				_hardTurnTarget = FRotator(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			FRotator interpControllerRotation = FMath::RInterpTo(currentControllerRotation, _hardTurnTarget, deltaSeconds, RushData.HardStopOrientationStrength);
			Controller->SetControlRotation(interpControllerRotation);
		}
	}
}
#pragma endregion


#pragma region Rush Action Sphere Timer Management
void ARushCharacter::SetRushTargetTimeScale(float timeScale)
{
	_targetRushTimeScale = timeScale;
}

void ARushCharacter::ResetRushTimeScale()
{
	_targetRushTimeScale = 1.0f;
}

void ARushCharacter::ExecuteRushTimeScaleUpdatePerTick(float DeltaSeconds)
{
	if (_targetRushTimeScale == -1.0f)
		return;

	UWorld* world = GetWorld();
	if (world != NULL)
	{
		float timeScale = FMath::FInterpTo(UGameplayStatics::GetGlobalTimeDilation(world), _targetRushTimeScale, DeltaSeconds, 8.0f);

		if (timeScale == _targetRushTimeScale)
		{
			_targetRushTimeScale = -1.0f;
		}

		UGameplayStatics::SetGlobalTimeDilation(world, timeScale);
	}
}
#pragma endregion