// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "CustomStyle/SuperManagerStyle.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();
	
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();  //注册Tab
}

#pragma region ContentBrowserMenuExtention
//第一层委托：注册右键菜单
void FSuperManagerModule::InitCBMenuExtention()
{
	//加载内容浏览器模块，拿到 FContentBrowserModule 引用
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//返回列表委托数组的引用，即默认菜单列表
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//创建委托类
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	//委托类加入列表委托数组
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);
	
	/*等效上面三行做法
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));*/
}

//第二次委托：描述“在哪个锚点后加东西”，负责构建菜单项
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());
	if (SelectedPaths.Num() > 0) {
		MenuExtender->AddMenuExtension(FName("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(), FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));

		FolderPathSelected = SelectedPaths;
	}
	
	return MenuExtender;
}

//第三层委托：每个菜单项点击后执行什么
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder.")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"), //自定义图标
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folders.")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"), //自定义图标
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting.")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"), //自定义图标
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathSelected.Num() > 1) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Type::Ok, TEXT("You can only do this one folder."));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathSelected[0]);
	if (AssetsPathNames.Num() == 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Type::Ok, TEXT("No asset found under selected folders."), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = 
	DebugHeader::ShowMsgDialog(EAppMsgType::Type::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + TEXT(" assets need to be checked.\nWould you like to procceed?"), false);
	if (ConfirmResult == EAppReturnType::Type::No) return;

	FixUpRedirectors(); //修复重定向器
	TArray<FAssetData> UnusedAssetsData;
	for (const FString& AssetPathName : AssetsPathNames) {
		if (AssetPathName.Contains(TEXT("Developers")) || 
			AssetPathName.Contains(TEXT("Collections")) || 
			AssetPathName.Contains(TEXT("__ExternalActors__")) || 
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
			continue;   //过滤根目录
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if (AssetReferencers.Num() == 0) {
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsData.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsData.Num() > 0) ObjectTools::DeleteAssets(UnusedAssetsData);
	else DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folders."), false);
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	//列出资产路径和目录路径
	TArray<FString> FolderPathArray = UEditorAssetLibrary::ListAssets(FolderPathSelected[0], true, true); //第一个True，递归子文件， 第二个，包括目录（false则只有资产）
	uint32 Counter = 0;

	FString EmptyFolderPathNames;
	TArray<FString> EmptyFolderPathsArray;

	for (const FString& FolderPath : FolderPathArray) {
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
			continue;   //过滤根目录

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue; //过滤掉资产路径，只保留目录路径
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath, true /*递归子目录*/)) {
			EmptyFolderPathNames.Append(FolderPath);
			EmptyFolderPathNames.Append(TEXT("\n"));

			EmptyFolderPathsArray.Add(FolderPath);
		}//判空
	}

	if (EmptyFolderPathsArray.Num() == 0) {
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder."), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in:\n") + EmptyFolderPathNames + TEXT("Would you like to delete all?"), false);
	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFolderPathsArray) {
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)) Counter++;
		else DebugHeader::PrintLog(TEXT("Failed to delete ") + EmptyFolderPath);
	}

	if (Counter > 0) DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders."));
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();

	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors() {
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

#pragma endregion

#pragma region CustomEditorTab
void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	/*拿到全局 Tab 管理器单例*/
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))//设置 Tab 的显示标题，可以和真实内部名字不同
		.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));
}

//Tab创建回调
TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return 
	SNew(SDockTab)
	.TabRole(ETabRole::NomadTab) //设置 Tab 的角色为 Nomad Tab
	[
		SNew(SAdvanceDeletionTab)
		.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		.CurrentSelectedFolder(FolderPathSelected[0])
	];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetData;
	TArray<FString> AssetPathNames = UEditorAssetLibrary::ListAssets(FolderPathSelected[0]);

	for (const FString& AssetPathName : AssetPathNames) {
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
			continue;   //过滤根目录

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue; //跳过无资源文件路径

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetData;
}

#pragma endregion

#pragma region ProccessDataForAdvanceDeletionTab
bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete) {
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0) return true;
	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsDataToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsDataToDelete) > 0) return true;
	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const auto& DataSharedPtr : AssetsDataToFilter) {
		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->GetObjectPathString());

		if (AssetReferencers.Num() == 0) {
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const auto& DataSharedPtr : AssetsDataToFilter) {
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const auto& DataSharedPtr : AssetsDataToFilter) {
		TArray<TSharedPtr<FAssetData>> OutAssetData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetData);

		if (OutAssetData.Num() <= 1) continue;

		for (const auto& SameNameData : OutAssetData) {
			if (SameNameData.IsValid()) {
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);

	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);
}

#pragma endregion

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));
	FSuperManagerStyle::ShutDown();
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)