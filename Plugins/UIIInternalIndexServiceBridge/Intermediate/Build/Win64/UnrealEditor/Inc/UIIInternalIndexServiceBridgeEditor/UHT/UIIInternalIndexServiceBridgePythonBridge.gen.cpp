// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UIIInternalIndexServiceBridgeEditor/Public/UIIInternalIndexServiceBridgePythonBridge.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUIIInternalIndexServiceBridgePythonBridge() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
UIIINTERNALINDEXSERVICEBRIDGEEDITOR_API UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge();
UIIINTERNALINDEXSERVICEBRIDGEEDITOR_API UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_NoRegister();
UPackage* Z_Construct_UPackage__Script_UIIInternalIndexServiceBridgeEditor();
// End Cross Module References

// Begin Class UUIIInternalIndexServiceBridgePythonBridge Function GetLatestUIIHandoffContractPath
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics
{
	struct UIIInternalIndexServiceBridgePythonBridge_eventGetLatestUIIHandoffContractPath_Parms
	{
		FString ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Returns the conventional latest UII handoff path. The file still has to\n// exist; callers receive a normal pipeline report if it is missing.\n" },
#endif
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgePythonBridge.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Returns the conventional latest UII handoff path. The file still has to\nexist; callers receive a normal pipeline report if it is missing." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventGetLatestUIIHandoffContractPath_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge, nullptr, "GetLatestUIIHandoffContractPath", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::UIIInternalIndexServiceBridgePythonBridge_eventGetLatestUIIHandoffContractPath_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04022401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::UIIInternalIndexServiceBridgePythonBridge_eventGetLatestUIIHandoffContractPath_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgePythonBridge::execGetLatestUIIHandoffContractPath)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FString*)Z_Param__Result=UUIIInternalIndexServiceBridgePythonBridge::GetLatestUIIHandoffContractPath();
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgePythonBridge Function GetLatestUIIHandoffContractPath

// Begin Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffAndBuildCatalog
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics
{
	struct UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms
	{
		FString ContractPath;
		FString OutReportPath;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Python/Editor wrapper for import plus IIS catalog rebuild.\n" },
#endif
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgePythonBridge.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Python/Editor wrapper for import plus IIS catalog rebuild." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge, nullptr, "ImportUIIHandoffAndBuildCatalog", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffAndBuildCatalog_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffAndBuildCatalog)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgePythonBridge::ImportUIIHandoffAndBuildCatalog(Z_Param_ContractPath,Z_Param_Out_OutReportPath);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffAndBuildCatalog

// Begin Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffBuildCatalogAndEmbeddings
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics
{
	struct UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms
	{
		FString ContractPath;
		int32 MaxEmbeddingJobs;
		FString OutReportPath;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Python/Editor wrapper for the bounded full local handoff pipeline.\n" },
#endif
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgePythonBridge.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Python/Editor wrapper for the bounded full local handoff pipeline." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FIntPropertyParams NewProp_MaxEmbeddingJobs;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs = { "MaxEmbeddingJobs", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, MaxEmbeddingJobs), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge, nullptr, "ImportUIIHandoffBuildCatalogAndEmbeddings", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffBuildCatalogAndEmbeddings)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY(FIntProperty,Z_Param_MaxEmbeddingJobs);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgePythonBridge::ImportUIIHandoffBuildCatalogAndEmbeddings(Z_Param_ContractPath,Z_Param_MaxEmbeddingJobs,Z_Param_Out_OutReportPath);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffBuildCatalogAndEmbeddings

// Begin Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffContract
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics
{
	struct UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms
	{
		FString ContractPath;
		FString OutReportPath;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Python/Editor wrapper for the import-only pipeline.\n" },
#endif
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgePythonBridge.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Python/Editor wrapper for the import-only pipeline." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge, nullptr, "ImportUIIHandoffContract", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::UIIInternalIndexServiceBridgePythonBridge_eventImportUIIHandoffContract_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffContract)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgePythonBridge::ImportUIIHandoffContract(Z_Param_ContractPath,Z_Param_Out_OutReportPath);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgePythonBridge Function ImportUIIHandoffContract

// Begin Class UUIIInternalIndexServiceBridgePythonBridge
void UUIIInternalIndexServiceBridgePythonBridge::StaticRegisterNativesUUIIInternalIndexServiceBridgePythonBridge()
{
	UClass* Class = UUIIInternalIndexServiceBridgePythonBridge::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GetLatestUIIHandoffContractPath", &UUIIInternalIndexServiceBridgePythonBridge::execGetLatestUIIHandoffContractPath },
		{ "ImportUIIHandoffAndBuildCatalog", &UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffAndBuildCatalog },
		{ "ImportUIIHandoffBuildCatalogAndEmbeddings", &UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffBuildCatalogAndEmbeddings },
		{ "ImportUIIHandoffContract", &UUIIInternalIndexServiceBridgePythonBridge::execImportUIIHandoffContract },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UUIIInternalIndexServiceBridgePythonBridge);
UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_NoRegister()
{
	return UUIIInternalIndexServiceBridgePythonBridge::StaticClass();
}
struct Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "UIIInternalIndexServiceBridgePythonBridge.h" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgePythonBridge.h" },
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_GetLatestUIIHandoffContractPath, "GetLatestUIIHandoffContractPath" }, // 2049561615
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffAndBuildCatalog, "ImportUIIHandoffAndBuildCatalog" }, // 3973329243
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffBuildCatalogAndEmbeddings, "ImportUIIHandoffBuildCatalogAndEmbeddings" }, // 4115750786
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgePythonBridge_ImportUIIHandoffContract, "ImportUIIHandoffContract" }, // 4104753837
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UUIIInternalIndexServiceBridgePythonBridge>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
	(UObject* (*)())Z_Construct_UPackage__Script_UIIInternalIndexServiceBridgeEditor,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::ClassParams = {
	&UUIIInternalIndexServiceBridgePythonBridge::StaticClass,
	nullptr,
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	0,
	0,
	0x001000A0u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::Class_MetaDataParams), Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge()
{
	if (!Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgePythonBridge.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgePythonBridge.OuterSingleton, Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgePythonBridge.OuterSingleton;
}
template<> UIIINTERNALINDEXSERVICEBRIDGEEDITOR_API UClass* StaticClass<UUIIInternalIndexServiceBridgePythonBridge>()
{
	return UUIIInternalIndexServiceBridgePythonBridge::StaticClass();
}
UUIIInternalIndexServiceBridgePythonBridge::UUIIInternalIndexServiceBridgePythonBridge(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UUIIInternalIndexServiceBridgePythonBridge);
UUIIInternalIndexServiceBridgePythonBridge::~UUIIInternalIndexServiceBridgePythonBridge() {}
// End Class UUIIInternalIndexServiceBridgePythonBridge

// Begin Registration
struct Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridgeEditor_Public_UIIInternalIndexServiceBridgePythonBridge_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UUIIInternalIndexServiceBridgePythonBridge, UUIIInternalIndexServiceBridgePythonBridge::StaticClass, TEXT("UUIIInternalIndexServiceBridgePythonBridge"), &Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgePythonBridge, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UUIIInternalIndexServiceBridgePythonBridge), 871566459U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridgeEditor_Public_UIIInternalIndexServiceBridgePythonBridge_h_3073037670(TEXT("/Script/UIIInternalIndexServiceBridgeEditor"),
	Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridgeEditor_Public_UIIInternalIndexServiceBridgePythonBridge_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridgeEditor_Public_UIIInternalIndexServiceBridgePythonBridge_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
