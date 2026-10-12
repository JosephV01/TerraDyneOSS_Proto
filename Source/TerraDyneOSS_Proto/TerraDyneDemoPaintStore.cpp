#include "TerraDyneDemoPaintStore.h"

#include "Engine/TextureRenderTarget2D.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "TerraDyneDemoTypes.h"
#include "UnrealClient.h"

void UTerraDyneDemoPaintStore::Initialize(int32 InResolution)
{
	Resolution = FMath::Max(2, InResolution);
	const int32 LayerCount = TerraDyneDemoLayers::Count();
	Weights.SetNumZeroed(LayerCount * Resolution * Resolution);
	FillLayer(0, 255);

	auto MakeRT = [this](const TCHAR* Name) -> UTextureRenderTarget2D*
	{
		UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this, MakeUniqueObjectName(this, UTextureRenderTarget2D::StaticClass(), Name));
		RT->RenderTargetFormat = RTF_RGBA8;
		RT->ClearColor = FLinearColor::Black;
		RT->bAutoGenerateMips = false;
		RT->InitAutoFormat(Resolution, Resolution);
		RT->UpdateResourceImmediate(true);
		return RT;
	};
	WeightRT0 = MakeRT(TEXT("RT_DemoPaint0"));
	WeightRT1 = MakeRT(TEXT("RT_DemoPaint1"));
	WeightRT2 = MakeRT(TEXT("RT_DemoPaint2"));
	bReady = true;
	UploadToRT();
}

int32 UTerraDyneDemoPaintStore::IndexAt(int32 X, int32 Y, int32 Layer) const
{
	if (X < 0 || Y < 0 || X >= Resolution || Y >= Resolution || Layer < 0 || Layer >= TerraDyneDemoLayers::Count())
	{
		return INDEX_NONE;
	}
	return Layer * Resolution * Resolution + Y * Resolution + X;
}

uint8* UTerraDyneDemoPaintStore::LayerData(int32 Layer)
{
	return Weights.GetData() + Layer * Resolution * Resolution;
}

const uint8* UTerraDyneDemoPaintStore::LayerData(int32 Layer) const
{
	return Weights.GetData() + Layer * Resolution * Resolution;
}

void UTerraDyneDemoPaintStore::FillLayer(int32 LayerIndex, uint8 Value)
{
	if (!Weights.Num())
	{
		return;
	}
	uint8* Data = LayerData(LayerIndex);
	if (!Data)
	{
		return;
	}
	FMemory::Memset(Data, Value, Resolution * Resolution);
}

bool UTerraDyneDemoPaintStore::ApplyPaint(float NormalizedX, float NormalizedY, float RadiusNorm, int32 LayerIndex, float Strength)
{
	if (!bReady || LayerIndex < 0 || LayerIndex >= TerraDyneDemoLayers::Count())
	{
		return false;
	}

	const float CX = FMath::Clamp(NormalizedX, 0.f, 1.f) * (Resolution - 1);
	const float CY = FMath::Clamp(NormalizedY, 0.f, 1.f) * (Resolution - 1);
	const float RadiusPx = FMath::Max(1.f, RadiusNorm * (Resolution - 1));
	const float RadiusSq = RadiusPx * RadiusPx;
	const int32 X0 = FMath::Max(0, FMath::FloorToInt(CX - RadiusPx));
	const int32 Y0 = FMath::Max(0, FMath::FloorToInt(CY - RadiusPx));
	const int32 X1 = FMath::Min(Resolution - 1, FMath::CeilToInt(CX + RadiusPx));
	const int32 Y1 = FMath::Min(Resolution - 1, FMath::CeilToInt(CY + RadiusPx));
	int32 Touched = 0;
	const float CoverScale = FMath::Clamp(Strength, 0.05f, 1.f);

	for (int32 Y = Y0; Y <= Y1; ++Y)
	{
		for (int32 X = X0; X <= X1; ++X)
		{
			const float DX = static_cast<float>(X) - CX;
			const float DY = static_cast<float>(Y) - CY;
			const float DistSq = DX * DX + DY * DY;
			if (DistSq > RadiusSq)
			{
				continue;
			}
			const float Falloff = 1.f - FMath::Sqrt(DistSq / RadiusSq);
			const float Smooth = Falloff * Falloff * (3.f - 2.f * Falloff);
			const float Cover = CoverScale * Smooth;
			for (int32 Layer = 0; Layer < TerraDyneDemoLayers::Count(); ++Layer)
			{
				uint8* Data = LayerData(Layer);
				const float Target = (Layer == LayerIndex) ? 255.f : 0.f;
				Data[Y * Resolution + X] = static_cast<uint8>(FMath::Clamp(
					FMath::RoundToInt(FMath::Lerp(static_cast<float>(Data[Y * Resolution + X]), Target, Cover)), 0, 255));
			}
			++Touched;
		}
	}

	if (Touched > 0)
	{
		UploadToRT();
	}
	return Touched > 0;
}

uint8 UTerraDyneDemoPaintStore::SampleWeight(int32 LayerIndex, float NormalizedX, float NormalizedY) const
{
	if (!bReady || LayerIndex < 0 || LayerIndex >= TerraDyneDemoLayers::Count())
	{
		return 0;
	}
	const int32 X = FMath::Clamp(FMath::RoundToInt(NormalizedX * (Resolution - 1)), 0, Resolution - 1);
	const int32 Y = FMath::Clamp(FMath::RoundToInt(NormalizedY * (Resolution - 1)), 0, Resolution - 1);
	return LayerData(LayerIndex)[Y * Resolution + X];
}

FLinearColor UTerraDyneDemoPaintStore::SampleColor(float NormalizedX, float NormalizedY) const
{
	FLinearColor Acc = FLinearColor::Black;
	float Sum = 0.f;
	const int32 X = FMath::Clamp(FMath::RoundToInt(NormalizedX * (Resolution - 1)), 0, Resolution - 1);
	const int32 Y = FMath::Clamp(FMath::RoundToInt(NormalizedY * (Resolution - 1)), 0, Resolution - 1);
	for (int32 Layer = 0; Layer < TerraDyneDemoLayers::Count(); ++Layer)
	{
		const uint8 W = LayerData(Layer)[Y * Resolution + X];
		if (W > 0)
		{
			Acc += TerraDyneDemoLayers::Color(Layer) * (W / 255.f);
			Sum += W / 255.f;
		}
	}
	return Sum > KINDA_SMALL_NUMBER ? Acc / Sum : TerraDyneDemoLayers::Color(0);
}

int32 UTerraDyneDemoPaintStore::DominantLayer(float NormalizedX, float NormalizedY) const
{
	const int32 X = FMath::Clamp(FMath::RoundToInt(NormalizedX * (Resolution - 1)), 0, Resolution - 1);
	const int32 Y = FMath::Clamp(FMath::RoundToInt(NormalizedY * (Resolution - 1)), 0, Resolution - 1);
	int32 Best = 0;
	uint8 BestW = 0;
	for (int32 Layer = 0; Layer < TerraDyneDemoLayers::Count(); ++Layer)
	{
		const uint8 W = LayerData(Layer)[Y * Resolution + X];
		if (W >= BestW)
		{
			BestW = W;
			Best = Layer;
		}
	}
	return Best;
}

void UTerraDyneDemoPaintStore::UploadToRT()
{
	if (!bReady || !WeightRT0)
	{
		return;
	}

	auto Upload = [&](UTextureRenderTarget2D* RT, int32 BaseLayer)
	{
		if (!RT)
		{
			return;
		}
		TArray<FColor> Pixels;
		Pixels.SetNumUninitialized(Resolution * Resolution);
		for (int32 Y = 0; Y < Resolution; ++Y)
		{
			for (int32 X = 0; X < Resolution; ++X)
			{
				auto Chan = [&](int32 Offset) -> uint8
				{
					const int32 Layer = BaseLayer + Offset;
					return Layer < TerraDyneDemoLayers::Count() ? LayerData(Layer)[Y * Resolution + X] : 0;
				};
				Pixels[Y * Resolution + X] = FColor(Chan(0), Chan(1), Chan(2), Chan(3));
			}
		}
		FTextureRenderTargetResource* Res = RT->GameThread_GetRenderTargetResource();
		if (!Res)
		{
			return;
		}
		FUpdateTextureRegion2D Region(0, 0, 0, 0, Resolution, Resolution);
		const int32 Bytes = Pixels.Num() * sizeof(FColor);
		uint8* Copy = static_cast<uint8*>(FMemory::Malloc(Bytes));
		FMemory::Memcpy(Copy, Pixels.GetData(), Bytes);
		ENQUEUE_RENDER_COMMAND(DemoPaintUpload)(
			[Res, Region, Copy, Stride = Resolution * static_cast<int32>(sizeof(FColor))](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.UpdateTexture2D(Res->GetRenderTargetTexture(), 0, Region, Stride, Copy);
				FMemory::Free(Copy);
			});
	};

	Upload(WeightRT0, 0);
	Upload(WeightRT1, 4);
	Upload(WeightRT2, 8);
}
