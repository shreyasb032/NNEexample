// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NNE.h"
#include "NNERuntimeCPU.h"
#include "NNEModelData.h"

#include "NNEInferenceActor.generated.h"

class FMyModelHelper
{
public:
	TUniquePtr<UE::NNE::IModelInstanceCPU> ModelInstance;
	TArray<float> InputData;
	TArray<float> OutputData;
	TArray<UE::NNE::FTensorBindingCPU> InputBindings;
	TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
	bool bIsRunning;
};

UCLASS()
class NNEEXAMPLE_API ANNEInferenceActor : public AActor
{
	GENERATED_BODY()

public:
	ANNEInferenceActor();

protected:
	virtual void BeginPlay() override;

public:
	//virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNNEModelData> PreLoadedModelData;

	//UPROPERTY(EditAnywhere)
	//TSoftObjectPtr<UNNEModelData> LazyLoadedModelData;

	UFUNCTION(BlueprintCallable)
	//void RunModel(const TArray<float>& InputData);
	void RunModel(const TArray<float>& InputData, TArray<float>& OutputData);
//	void RunModel(const TArray<float>& InputData, UPARAM(ref) TArray<float>& OutputData);

private:
	TSharedPtr<FMyModelHelper> ModelHelper;
};
