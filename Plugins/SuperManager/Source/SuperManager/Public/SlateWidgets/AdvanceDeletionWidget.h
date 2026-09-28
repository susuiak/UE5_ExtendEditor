#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget {
	//Slate 控件的参数声明宏，用来定义这个控件可以被外部传入哪些参数
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)

	SLATE_ARGUMENT(FString, CurrentSelectedFolder)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetData; //将传入的资源数据（AssetData）存下
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;  //存下构建好的复选框
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;

	//创建滚动框
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;  //存下已构建的滚动框指针
	void RefreshAssetListView(); //刷新滚动框

#pragma region ComboBoxForListingCondition
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();  //构造
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;	
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);  //构造回调
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);  //状态变化回调

	TSharedPtr<STextBlock> ComboDisplayTextBlock;  //用SAssignNew，保存创建下来的Slate控件，之后，根据选择选项改变该控件标题。
	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify);
#pragma endregion

#pragma region RowWidgetForAssetData
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable);

	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);

	//创建复选框
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	//复选框状态变化回调
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay); //创建按钮控件
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData); //按钮回调
#pragma endregion

#pragma region TabButtons
	TSharedRef<SButton> ConstructDeleteAllButton();
	FReply OnDeleteAllButtonClicked();

	TSharedRef<SButton> ConstructSelectAllButton();
	FReply OnSelectAllButtonClicked();

	TSharedRef<SButton> ConstructDeselectAllButton();
	FReply OnDeselectAllButtonClicked();


	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion

	////FCoreStyle:Slate 的默认核心样式集，提供引擎内置的基础样式，比如字体、图标、颜色。Get() 返回这个样式集的引用
	FSlateFontInfo GetEmboseedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); } 

};