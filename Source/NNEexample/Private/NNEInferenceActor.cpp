#include "NNEInferenceActor.h"

#include "Engine/AssetManager.h"

ANNEInferenceActor::ANNEInferenceActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANNEInferenceActor::BeginPlay()
{
	Super::BeginPlay();

	// Example for manual loading
	//TObjectPtr<UNNEModelData> ManuallyLoadedModelData = LoadObject<UNNEModelData>(GetTransientPackage(), TEXT("/Game/mnist-8.mnist-8"));
	//if (ManuallyLoadedModelData)
	//{
	//	UE_LOG(LogTemp, Display, TEXT("ManuallyLoadedModelData loaded %s"), *ManuallyLoadedModelData->GetName());
	//	// You can use ManuallyLoadedModelData here to create a model and corresponding model instance(s)
	//	// ManuallyLoadedModelData will be unloaded when the current function returns
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("ManuallyLoadedModelData is not loaded, please check the static path to your asset"));
	//}

	// Example for automated loading
	if (PreLoadedModelData)
	{
		UE_LOG(LogTemp, Display, TEXT("PreLoadedModelData loaded %s"), *PreLoadedModelData->GetName());
		// You can use PreLoadedModelData here to create a model and corresponding model instance(s)
		// PreLoadedModelData will be unloaded when the owning object or actor unloads
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PreLoadedModelData is not set, please assign it in the editor"));
	}

	// First part of the example for delayed loading
	/*if (LazyLoadedModelData.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("LazyLoadedModelData is not set, please assign it in the editor"));
	}
	else
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(LazyLoadedModelData.ToSoftObjectPath());
	}*/

	if (PreLoadedModelData)
	{
		// Example for model creation
		TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(FString("NNERuntimeORTCpu"));
		if (Runtime.IsValid())
		{
			ModelHelper = MakeShared<FMyModelHelper>();

			TUniquePtr<UE::NNE::IModelCPU> Model = Runtime->CreateModel(PreLoadedModelData);
			if (Model.IsValid())
			{
				ModelHelper->ModelInstance = Model->CreateModelInstance();
				if (ModelHelper->ModelInstance.IsValid())
				{
					ModelHelper->bIsRunning = false;

					// Example for querying and testing in- and outputs
					TConstArrayView<UE::NNE::FTensorDesc> InputTensorDescs = ModelHelper->ModelInstance->GetInputTensorDescs();
					checkf(InputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single input tensor"));
					UE::NNE::FSymbolicTensorShape SymbolicInputTensorShape = InputTensorDescs[0].GetShape();
					TArray<UE::NNE::FTensorShape> InputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicInputTensorShape) };
					/*FString out = "(";
					for (uint32 d : SymbolicInputTensorShape.GetData())
					{
						out += FString::FromInt(d);
						out += ",";
					}
					out += ")";
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, out);
					return;*/

					checkf(SymbolicInputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable input tensor dimensions"));
					ModelHelper->ModelInstance->SetInputTensorShapes(InputTensorShapes);

					TConstArrayView<UE::NNE::FTensorDesc> OutputTensorDescs = ModelHelper->ModelInstance->GetOutputTensorDescs();
					checkf(OutputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single output tensor"));
					UE::NNE::FSymbolicTensorShape SymbolicOutputTensorShape = OutputTensorDescs[0].GetShape();
					checkf(SymbolicOutputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable output tensor dimensions"));
					TArray<UE::NNE::FTensorShape> OutputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicOutputTensorShape) };

					// Example for creating in- and outputs
					ModelHelper->InputData.SetNumZeroed(InputTensorShapes[0].Volume());
					ModelHelper->InputBindings.SetNumZeroed(1);
					ModelHelper->InputBindings[0].Data = ModelHelper->InputData.GetData();
					ModelHelper->InputBindings[0].SizeInBytes = ModelHelper->InputData.Num() * sizeof(float);

					ModelHelper->OutputData.SetNumZeroed(OutputTensorShapes[0].Volume());
					ModelHelper->OutputBindings.SetNumZeroed(1);
					ModelHelper->OutputBindings[0].Data = ModelHelper->OutputData.GetData();
					ModelHelper->OutputBindings[0].SizeInBytes = ModelHelper->OutputData.Num() * sizeof(float);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create the model instance"));
					ModelHelper.Reset();
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create the model"));
				ModelHelper.Reset();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot find runtime NNERuntimeORTCpu, please enable the corresponding plugin"));
		}
	}
}

//void ANNEInferenceActor::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//	// Second part of the example for delayed loading
//	if (LazyLoadedModelData.IsValid())
//	{
//		UE_LOG(LogTemp, Display, TEXT("LazyLoadedModelData loaded %s"), *LazyLoadedModelData.Get()->GetName());
//		// You can use LazyLoadedModelData here to create a model and corresponding model instance(s)
//		// LazyLoadedModelData will be unloaded in the next line
//		LazyLoadedModelData.Reset();
//	}
//
//	if (ModelHelper.IsValid())
//	{
//		// Example for async inference
//		if (!ModelHelper->bIsRunning)
//		{
//			// Process ModelHelper->OutputData from the previous run here
//			// Fill in new data into ModelHelper->InputData here
//
//			ModelHelper->bIsRunning = true;
//			TSharedPtr<FMyModelHelper> ModelHelperPtr = ModelHelper;
//			AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [ModelHelperPtr]()
//				{
//					if (ModelHelperPtr->ModelInstance->RunSync(ModelHelperPtr->InputBindings, ModelHelperPtr->OutputBindings) != 0)
//					{
//						UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
//					}
//					AsyncTask(ENamedThreads::GameThread, [ModelHelperPtr]()
//						{
//							ModelHelperPtr->bIsRunning = false;
//						});
//				});
//
//		}
//	}
//}

//void ANNEInferenceActor::RunModel(const TArray<float>& InputData, UPARAM(ref) TArray<float>& OutputData)
void ANNEInferenceActor::RunModel(const TArray<float>& InputData, TArray<float>& OutputData)
{
	//// Example for creating in- and outputs
	//ModelHelper->InputData = InputData;
	//ModelHelper->InputBindings.SetNumZeroed(1);
	//ModelHelper->InputBindings[0].Data = ModelHelper->InputData.GetData();
	//ModelHelper->InputBindings[0].SizeInBytes = ModelHelper->InputData.Num() * sizeof(float);

	//ModelHelper->OutputData = OutputData;
	//ModelHelper->OutputBindings.SetNumZeroed(1);
	//ModelHelper->OutputBindings[0].Data = ModelHelper->OutputData.GetData();
	//ModelHelper->OutputBindings[0].SizeInBytes = ModelHelper->OutputData.Num() * sizeof(float);
	//int i = 0;
	//for (float& val : ModelHelper->InputData)
	//{
	//	val = (float) i;
	//	i++;
	//}
	ModelHelper->InputData = InputData;

	//TConstArrayView<UE::NNE::FTensorDesc> InputTensorDescs = ModelHelper->ModelInstance->GetInputTensorDescs();
	//checkf(InputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single input tensor"));
	//UE::NNE::FSymbolicTensorShape SymbolicInputTensorShape = InputTensorDescs[0].GetShape();
	//TArray<UE::NNE::FTensorShape> InputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicInputTensorShape) };

	//checkf(SymbolicInputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable input tensor dimensions"));
	//ModelHelper->ModelInstance->SetInputTensorShapes(InputTensorShapes);

	//TConstArrayView<UE::NNE::FTensorDesc> OutputTensorDescs = ModelHelper->ModelInstance->GetOutputTensorDescs();
	//checkf(OutputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single output tensor"));
	//UE::NNE::FSymbolicTensorShape SymbolicOutputTensorShape = OutputTensorDescs[0].GetShape();
	//checkf(SymbolicOutputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable output tensor dimensions"));
	//TArray<UE::NNE::FTensorShape> OutputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicOutputTensorShape) };

	// Example for creating in- and outputs
	//ModelHelper->InputData.SetNumZeroed(InputTensorShapes[0].Volume());
	//ModelHelper->InputBindings.SetNumZeroed(1);
	//ModelHelper->InputBindings[0].Data = ModelHelper->InputData.GetData();
	//ModelHelper->InputBindings[0].SizeInBytes = ModelHelper->InputData.Num() * sizeof(float);

	//ModelHelper->OutputData.SetNumZeroed(OutputTensorShapes[0].Volume());
	//ModelHelper->OutputBindings.SetNumZeroed(1);
	//ModelHelper->OutputBindings[0].Data = ModelHelper->OutputData.GetData();
	//ModelHelper->OutputBindings[0].SizeInBytes = ModelHelper->OutputData.Num() * sizeof(float);

	//ModelHelper->ModelInstance->RunSync(ModelHelper->InputBindings, ModelHelper->OutputBindings);
	//OutputData = ModelHelper->OutputBindings[0].Data;

	if (ModelHelper.IsValid())
	{
		if (ModelHelper->ModelInstance->RunSync(ModelHelper->InputBindings, ModelHelper->OutputBindings) != 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
		}

		//// Example for async inference
		//if (!ModelHelper->bIsRunning)
		//{
		//	// Process ModelHelper->OutputData from the previous run here
		//	// Fill in new data into ModelHelper->InputData here

		//	ModelHelper->bIsRunning = true;
		//	TSharedPtr<FMyModelHelper> ModelHelperPtr = ModelHelper;
		//	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [ModelHelperPtr]()
		//		{
		//			if (ModelHelperPtr->ModelInstance->RunSync(ModelHelperPtr->InputBindings, ModelHelperPtr->OutputBindings) != 0)
		//			{
		//				UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
		//			}
		//			AsyncTask(ENamedThreads::GameThread, [ModelHelperPtr]()
		//				{
		//					ModelHelperPtr->bIsRunning = false;
		//				});
		//		});
		//}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("ModelHelper is not valid"));
	}

	for (float val : ModelHelper->InputData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, "Input: " + FString::SanitizeFloat(val));
	}

	/*for (float val : ModelHelper->OutputData)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "Output: " + FString::SanitizeFloat(val));
	}*/
	OutputData = ModelHelper->OutputData;

}