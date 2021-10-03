// Copyright Epic Games, Inc. All Rights Reserved.

#include "StairDismountCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
//////////////////////////////////////////////////////////////////////////
// AStairDismountCharacter

AStairDismountCharacter::AStairDismountCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ForceDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForceDirection"));
	ForceDirectionArrow->bHiddenInGame=false;
	ForceDirectionArrow->SetVisibility(true);
	ForceDirectionArrow->SetupAttachment(GetRootComponent());

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	MeshPrivate=nullptr;
	DefaultMeshTransform =GetMesh()?GetMesh()->GetRelativeTransform() : FTransform::Identity;

	LinearVelocityThreshold=5.f;
	AngularVelocityThreshold=5.f;

	LinearVelocityDmgThresholdSquared=LinearVelocityThreshold*LinearVelocityThreshold;
	AngularVelocityDmgThresholdSquared= AngularVelocityThreshold*AngularVelocityThreshold;

	DamageScalar=0.05f;
	bIsRagdolling=false;
}

void AStairDismountCharacter::RagdollCharacter( const float ImpulsePower)
{
	check(MeshPrivate);
	MeshPrivate->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// so the hit events trigger with world objects.
	MeshPrivate->SetCollisionObjectType(ECC_WorldDynamic);

	MeshPrivate->SetAllBodiesBelowSimulatePhysics(FName("Pelvis"),true);
	MeshPrivate->AddImpulse(GetArrowForceDirection() * ImpulsePower,TargetBone);
	bIsRagdolling=true;
}

void AStairDismountCharacter::ResetCharacater()
{
	MeshPrivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshPrivate->SetAllBodiesBelowSimulatePhysics(FName("pelvis"),false);
	bIsRagdolling=false;
	// reattach mesh to capsule component after physics sim
	if(!IsValid(MeshPrivate->GetAttachParent()))
	{
		MeshPrivate->AttachToComponent(GetCapsuleComponent(),FAttachmentTransformRules::KeepWorldTransform);
		MeshPrivate->SetRelativeLocationAndRotation(DefaultMeshTransform.GetTranslation(), DefaultMeshTransform.GetRotation());
	}
	TotalDamageTaken=0;
	BP_OnCharacterReset();
}

void AStairDismountCharacter::BP_SetLinearVelocityThreshold(const float Value)
{
	LinearVelocityThreshold = Value >0.0f ? Value : 0.0f ;
	LinearVelocityDmgThresholdSquared = LinearVelocityThreshold*LinearVelocityThreshold;
}

void AStairDismountCharacter::BP_SetAngularVelocityThreshold(const float Value)
{
	AngularVelocityThreshold = Value > 0.0f ? Value : 0.0f;
	AngularVelocityDmgThresholdSquared = AngularVelocityThreshold * AngularVelocityThreshold;
}

void AStairDismountCharacter::SetForceDirectionArrow(const FVector Dir, const FVector Location, const FName BoneName)
{
	check(ForceDirectionArrow);

	TargetBone = BoneName;
	FVector OffsetLocation = Location;
	OffsetLocation += (-Dir * ForceDirectionArrow->ArrowLength);
	ForceDirectionArrow->SetWorldLocation(OffsetLocation);
	ForceDirectionArrow->SetWorldRotation(FRotationMatrix::MakeFromXZ(Dir,FVector::UpVector).ToQuat());
}

FORCEINLINE FVector AStairDismountCharacter::GetArrowForceDirection() const
{
	return ForceDirectionArrow->GetForwardVector();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AStairDismountCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AStairDismountCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStairDismountCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AStairDismountCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AStairDismountCharacter::LookUpAtRate);


}


bool AStairDismountCharacter::GetIsRagdolling() const
{
	return bIsRagdolling;
}

void AStairDismountCharacter::BeginPlay()
{
	Super::BeginPlay();
	if(GetMesh())
	{
		DefaultMeshTransform = GetMesh()->GetRelativeTransform();
		GetMesh()->OnComponentHit.AddDynamic(this,&AStairDismountCharacter::OnMeshHit);
		MeshPrivate = GetMesh();
	}
}

void AStairDismountCharacter::PossessedBy(AController* NewController)
{	
	Super::PossessedBy(NewController);
	if(NewController && NewController->IsLocalController())
	{
		//UWidgetBlueprintLibrary::SetInputMode_GameAndUI
		APlayerController* Pc = CastChecked<APlayerController>(NewController);
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		Pc->bShowMouseCursor =  true;
		Pc->SetInputMode(InputMode);
	}
}

void AStairDismountCharacter::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(!Hit.BoneName.IsNone())
	{
		check(MeshPrivate);

		const FVector Angular_V = MeshPrivate->GetPhysicsAngularVelocityInDegrees(Hit.BoneName);
		const FVector Linear_V = MeshPrivate->GetPhysicsLinearVelocity(Hit.BoneName);

		float DamageTaken=0.0f;

		const float AngularSqrd = Angular_V.SizeSquared();
		const float LinearSqrd = Linear_V.SizeSquared();

		if(AngularSqrd >= AngularVelocityDmgThresholdSquared)
		{
			DamageTaken += AngularSqrd;
		}
		if(LinearSqrd >= LinearVelocityDmgThresholdSquared)
		{
			DamageTaken+= LinearSqrd;
		}
		DamageTaken*= DamageScalar;

		if(DamageTaken >0.01f)
		{
			const int32 NewDmg = FMath::TruncToInt(FMath::Sqrt(DamageTaken));
			TotalDamageTaken+= NewDmg;

			BP_OnBoneTakeDamage(Hit.BoneName, NewDmg);
		}
	}
}

void AStairDismountCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AStairDismountCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AStairDismountCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AStairDismountCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
