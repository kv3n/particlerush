// Some copyright should be here...
#pragma once

#include "GenerationBoxActor.generated.h"

class ULevelDesignerAsset;

struct FCreationData
{
	FVector MinBoundsValue;
	FVector MaxBoundsValue;
	FVector BoxExtent;
};

/**  */
UCLASS(BlueprintType)
class AGenerationBoxActor : public AActor
{
	GENERATED_BODY()

	AGenerationBoxActor();

	~AGenerationBoxActor();

#pragma region COMPONENT DECLERATIONS
protected:
	class USceneComponent* RootSceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Category = "Generation Box"))
	class UBoxComponent* GenerationBox;
#pragma endregion
	
#pragma region PROPERTY DECLERATIONS
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Category = "Generation Params", UIMin = 10.0f, UIMax = 10000.0f))
	float MinAlleySpacing;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Category = "Generation Params", UIMin = 10.0f, UIMax = 10000.0f))
	float MaxAlleySpacing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Category = "Generation Settings"))
	class ULevelDesignerAsset* LevelDesigner_SettingsAsset;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Meta = (Category = "Native Only"))
	bool IsPopulatedBefore;
#pragma endregion

#pragma region BASE CLASS OVERRIDES
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostEditMove(bool bFinished) override;

	virtual void Destroyed() override;
#pragma endregion


#pragma region PRIVATE HELPERS
private:
	FCreationData CreationTimeData;

	TArray<AActor*> GeneratedActors;

	void GetGenerationBounds(FCreationData& creationData);

	void ClearGeneratedActors();

	void ClearGeneratedActorsWithColor(const FColor& ActorClassColor);

	void PopulateWorld();

	void MoveGeneratedActorsToNewLocation(FVector movementOffset);

	FVector SpawnActor(UClass* ActorClass, const FVector& ActorLocation, const FRotator& ActorOrienation, const FColor& ActorClassColor, UStaticMesh* ActorMesh = NULL);
#pragma endregion


#pragma region PUBLIC INTERFACES
public:
	bool RemoveSpawnedActor(AActor* aAcorToRemove);
#pragma endregion
};