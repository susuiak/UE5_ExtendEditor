// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"


// 蓝图
#include "Engine/Blueprint.h"

// 材质
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialParameterCollection.h"

// 纹理
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"

// 粒子
#include "Particles/ParticleSystem.h"

// 静态网格 / 骨骼网格
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"

// 动画
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimBlueprint.h"

// 声音
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"

// 数据资产
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"

// 用户界面
#include "Blueprint/UserWidget.h"
#include "WidgetBlueprint.h"

//Niagara
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"

// 其他常见
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/Font.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Animation/Skeleton.h"
#include "Engine/CurveTable.h"
#include "Engine/SubsurfaceProfile.h"

#include "QuickAssetAction.generated.h"



UCLASS()
class SUPERMANAGER_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumOfDuplicates);
	
	UFUNCTION(CallInEditor)
	void AddPrefixes();

    UFUNCTION(CallInEditor)
    void RemoveUnusedAssets();

private:
    //前缀表
    TMap<UClass*, FString> PrefixMap = {
        // 蓝图
        {UBlueprint::StaticClass(),             TEXT("BP_")},

        // 材质
        {UMaterial::StaticClass(),              TEXT("M_")},
        {UMaterialInstanceConstant::StaticClass(), TEXT("MI_")},
        {UMaterialFunction::StaticClass(),      TEXT("MF_")},
        {UMaterialParameterCollection::StaticClass(), TEXT("MPC_")},

        // 纹理
        {UTexture2D::StaticClass(),             TEXT("T_")},
        {UTextureRenderTarget2D::StaticClass(), TEXT("RT_")},

        // 粒子
        {UParticleSystem::StaticClass(),        TEXT("PS_")},

        // 静态网格 / 骨骼网格
        {UStaticMesh::StaticClass(),            TEXT("SM_")},
        {USkeletalMesh::StaticClass(),          TEXT("SK_")},

        // 动画
        {UAnimSequence::StaticClass(),          TEXT("AS_")},
        {UAnimMontage::StaticClass(),           TEXT("AM_")},
        {UAnimBlueprint::StaticClass(),         TEXT("ABP_")},

        // 声音
        {USoundCue::StaticClass(),              TEXT("SC_")},
        {USoundWave::StaticClass(),             TEXT("SW_")},

        // 数据
        {UDataAsset::StaticClass(),             TEXT("DA_")},
        {UDataTable::StaticClass(),             TEXT("DT_")},

        // UI
        {UWidgetBlueprint::StaticClass(),       TEXT("WBP_")},

        // 物理
        {UPhysicsAsset::StaticClass(),          TEXT("PHYS_")},

        // 骨架
        {USkeleton::StaticClass(),              TEXT("SKEL_")},

        // 字体
        {UFont::StaticClass(),                  TEXT("F_")},

        // Niagara
        {UNiagaraSystem::StaticClass(),         TEXT("NS_")},
        {UNiagaraEmitter::StaticClass(),        TEXT("NE_")},
    };

    void FixUpRedirectors();
};
