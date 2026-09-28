// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention
	void InitCBMenuExtention();
	TArray<FString> FolderPathSelected; //保存选中的文件夹
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths /*选中文件夹数量*/);
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder); //菜单描述回调
	
	void OnDeleteUnusedAssetButtonClicked(); //删除资源回调函数
	void OnDeleteEmptyFoldersButtonClicked(); //删除空文件夹
	void OnAdvanceDeletionButtonClicked(); //高级删除

	void FixUpRedirectors(); //修复重定向器
#pragma endregion

#pragma region CustomEditor
	void RegisterAdvanceDeletionTab(); //注册 Tab Spawner

	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder(); //获取选定文件夹下资源数据

#pragma endregion

public:

#pragma region ProccessDataForAdvanceDeletionTab
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsDataToDelete);
	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);
	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

#pragma endregion
};
