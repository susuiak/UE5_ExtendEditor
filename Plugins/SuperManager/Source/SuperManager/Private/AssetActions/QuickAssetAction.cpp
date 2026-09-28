#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"


//复制资产
void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const auto& SelectedAssetData : SelectedAssetsData) {
		for (int32 i = 0; i < NumOfDuplicates; i++) {
			const FString SourceAssetPath = SelectedAssetData.GetObjectPathString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName)) {
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				Counter++;
			}
		}
	}

	if (Counter > 0) {
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated ") + FString::FromInt(Counter) + TEXT(" files"));
	}
}

//添加前缀
void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects) {
		if (!SelectedObject) continue;

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty()) {
			DebugHeader::Print(TEXT("Failed to find prefix for class") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		//判断是否已有前缀
		FString OldName = SelectedObject->GetName();
		if (OldName.StartsWith(*PrefixFound)) {
			DebugHeader::Print(OldName + TEXT(" already has prefix added."), FColor::Red);
			continue;
		}

		//判断是否为材质实例
		if (SelectedObject->IsA<UMaterialInstance>()) {
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewName = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewName);
		Counter++;
	}

	if (Counter > 0) DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + " assets");
}

//删除资产
void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	FixUpRedirectors();

	for (const auto& SelectedAssetData : SelectedAssetsData) {
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString(), false);

		if (AssetReferencers.Num() == 0) UnusedAssetsData.Add(SelectedAssetData);
	}

	if (UnusedAssetsData.Num() == 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}

	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if (NumOfAssetsDeleted == 0) return;
	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets"));
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;
	
	//加载AssetRegistry模块
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//构建过滤器
	FARFilter Filter;
	Filter.bRecursivePaths = true;   //递归扫描子目录
	Filter.PackagePaths.Emplace("/Game");	//从 /Game 根目录开始
	Filter.ClassPaths.Emplace(UObjectRedirector::StaticClass()->GetClassPathName());	//只查 UObjectRedirector 类型的资产
	/*这里用的是类名字符串 "ObjectRedirector"，而不是 UObjectRedirector::StaticClass()。
	  这是因为 AssetRegistry 的过滤器在资产还没加载时只能靠类名匹配*/

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	
	for (const auto& RedirectorData : OutRedirectors) {
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset())) {
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	if (RedirectorsToFixArray.Num() == 0) return;

	/*FAssetToolsModule 提供资产操作的高级 API。
	  FixupReferencers 接收一组重定向器，把所有指向这些重定向器的引用改为直接指向目标资产，然后删除重定向器。*/
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}
