// Fill out your copyright notice in the Description page of Project Settings.


#include "Explodeable.h"

// Sets default values
AExplodeable::AExplodeable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh= CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	if(TriggerSphere && GetRootComponent())
	{
		TriggerSphere->SetupAttachment(GetRootComponent());
		TriggerSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
		TriggerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic,ECollisionResponse::ECR_Overlap);
	}
	
}

// Called when the game starts or when spawned
void AExplodeable::BeginPlay()
{
	Super::BeginPlay();
	if(TriggerSphere)
	{
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this,&AExplodeable::OnTriggerHit);
	}
}

void AExplodeable::OnTriggerHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherComp && OtherComp->IsSimulatingPhysics())
	{
		Explode(OtherComp);
	}
}

void AExplodeable::Explode(UPrimitiveComponent* OtherComp)
{
	if(OtherComp)
	{
		OtherComp->AddImpulse((GetActorLocation()-OtherComp->GetComponentTransform().GetLocation()).GetSafeNormal() *ExplosiveForce );
	}


	OnExplode();

	FTimerDelegate Delegate;
	Delegate.BindLambda([this]{
		this->Destroy();
	});
	GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
}

// Called every frame
void AExplodeable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

