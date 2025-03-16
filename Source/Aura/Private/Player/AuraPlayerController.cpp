// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	/**
	* Line trace from cursor. There are several scenarios:
	* A. LastActor is null && ThisAcotr is null
	*	- Do nothing
	* B. LastActor is null && ThisActor is valid
	*	- Highlight ThisActor
	* C. LastActor is valid && ThisActor is null
	*	- UnHighlight LastActor
	* D. Both actors are valid, but LastActor != ThisActor
	*	- UnHighlight LastActor, and Highlight This Actor
	* E. Both actors are valid, and are the same actor
	*	- Do nothing
	**/

	if (LastActor == nullptr)
	{
		if (ThisActor.GetInterface())
		{
			UObject* Object = ThisActor.GetObject();
			IEnemyInterface* EnemyInterface = Cast<IEnemyInterface>(Object);

			if (EnemyInterface)
			{
				EnemyInterface->HighlightActor();
			}
		}
	}
	else // LastActor가 nullptr이 아닐 때
	{
		// LastActor가 유효한지 확인
		if (LastActor.GetInterface())
		{
			if (ThisActor.GetInterface() == nullptr)
			{
				LastActor->UnHighlightActor();
			}
			else // ThisActor도 유효할 때
			{
				LastActor->UnHighlightActor();
				UObject* Object = ThisActor.GetObject();
				IEnemyInterface* EnemyInterface = Cast<IEnemyInterface>(Object);

				if (EnemyInterface)
				{
					EnemyInterface->HighlightActor();
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LastActor is not valid."));
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}