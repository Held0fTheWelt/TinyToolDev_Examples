// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UIIInternalIndexServiceBridge/Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUIIInternalIndexServiceBridgeBlueprintLibrary() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
UIIINTERNALINDEXSERVICEBRIDGE_API UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary();
UIIINTERNALINDEXSERVICEBRIDGE_API UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_NoRegister();
UPackage* Z_Construct_UPackage__Script_UIIInternalIndexServiceBridge();
// End Cross Module References

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function GetLatestUIIHandoffContractPath
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventGetLatestUIIHandoffContractPath_Parms
	{
		FString ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Get Latest UII IIS Handoff Contract Path" },
		{ "Keywords", "UII IIS Handoff Contract Path" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Returns the conventional latest UII -> IIS handoff contract path under Saved/UnrealIntegrationIntelligence/iis_handoff." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventGetLatestUIIHandoffContractPath_Parms, ReturnValue), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "GetLatestUIIHandoffContractPath", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventGetLatestUIIHandoffContractPath_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventGetLatestUIIHandoffContractPath_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execGetLatestUIIHandoffContractPath)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FString*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::GetLatestUIIHandoffContractPath();
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function GetLatestUIIHandoffContractPath

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffAndBuildCatalog
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms
	{
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Import Latest UII Handoff and Build IIS Catalog" },
		{ "Keywords", "UII IIS Import Latest Handoff Catalog" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Imports the latest conventional UII handoff into IIS and rebuilds the IIS chunk catalog." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportLatestUIIHandoffAndBuildCatalog", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffAndBuildCatalog_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffAndBuildCatalog)
{
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportLatestUIIHandoffAndBuildCatalog(Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffAndBuildCatalog

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffBuildCatalogAndEmbeddings
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms
	{
		int32 MaxEmbeddingJobs;
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Run Latest Full UII IIS Handoff Pipeline" },
		{ "Keywords", "UII IIS Latest Full Handoff Pipeline Embeddings" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Runs the full bounded UII -> IIS pipeline for the latest conventional handoff contract." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_MaxEmbeddingJobs;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs = { "MaxEmbeddingJobs", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms, MaxEmbeddingJobs), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportLatestUIIHandoffBuildCatalogAndEmbeddings", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffBuildCatalogAndEmbeddings_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffBuildCatalogAndEmbeddings)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_MaxEmbeddingJobs);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportLatestUIIHandoffBuildCatalogAndEmbeddings(Z_Param_MaxEmbeddingJobs,Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffBuildCatalogAndEmbeddings

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffContract
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms
	{
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Import Latest UII Handoff into IIS" },
		{ "Keywords", "UII IIS Import Latest Handoff Evidence" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Imports the latest conventional UII handoff contract into IIS." },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportLatestUIIHandoffContract", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportLatestUIIHandoffContract_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffContract)
{
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportLatestUIIHandoffContract(Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportLatestUIIHandoffContract

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffAndBuildCatalog
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms
	{
		FString ContractPath;
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Import UII Handoff and Build IIS Catalog" },
		{ "Keywords", "UII IIS Import Handoff Catalog" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Imports the UII handoff into IIS and rebuilds the IIS chunk catalog." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportUIIHandoffAndBuildCatalog", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffAndBuildCatalog_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffAndBuildCatalog)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportUIIHandoffAndBuildCatalog(Z_Param_ContractPath,Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffAndBuildCatalog

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffBuildCatalogAndEmbeddings
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms
	{
		FString ContractPath;
		int32 MaxEmbeddingJobs;
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Run Full UII IIS Handoff Pipeline" },
		{ "Keywords", "UII IIS Full Handoff Pipeline Embeddings" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Imports the UII handoff, builds the IIS catalog, executes a bounded number of IIS embedding jobs, runs smoke retrieval, and writes agent contracts." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FIntPropertyParams NewProp_MaxEmbeddingJobs;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs = { "MaxEmbeddingJobs", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, MaxEmbeddingJobs), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_MaxEmbeddingJobs,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportUIIHandoffBuildCatalogAndEmbeddings", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffBuildCatalogAndEmbeddings_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffBuildCatalogAndEmbeddings)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY(FIntProperty,Z_Param_MaxEmbeddingJobs);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportUIIHandoffBuildCatalogAndEmbeddings(Z_Param_ContractPath,Z_Param_MaxEmbeddingJobs,Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffBuildCatalogAndEmbeddings

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffContract
struct Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics
{
	struct UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms
	{
		FString ContractPath;
		FString OutReportPath;
		TArray<FString> OutWarnings;
		bool ReturnValue;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "CallInEditor", "true" },
		{ "Category", "UII|Internal Index Service Bridge" },
		{ "DisplayName", "Import UII Handoff Contract into IIS" },
		{ "Keywords", "UII IIS Import Handoff Evidence" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Imports prepared chunks referenced by a UII handoff contract into IIS and writes a docking report." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ContractPath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FStrPropertyParams NewProp_ContractPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutReportPath;
	static const UECodeGen_Private::FStrPropertyParams NewProp_OutWarnings_Inner;
	static const UECodeGen_Private::FArrayPropertyParams NewProp_OutWarnings;
	static void NewProp_ReturnValue_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ContractPath = { "ContractPath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms, ContractPath), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ContractPath_MetaData), NewProp_ContractPath_MetaData) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutReportPath = { "OutReportPath", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms, OutReportPath), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutWarnings_Inner = { "OutWarnings", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutWarnings = { "OutWarnings", nullptr, (EPropertyFlags)0x0010000000000180, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms, OutWarnings), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
void Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit(void* Obj)
{
	((UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms*)Obj)->ReturnValue = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms), &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ContractPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutReportPath,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutWarnings_Inner,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_OutWarnings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::NewProp_ReturnValue,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, nullptr, "ImportUIIHandoffContract", nullptr, nullptr, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::PropPointers), sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::Function_MetaDataParams), Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::Function_MetaDataParams) };
static_assert(sizeof(Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::UIIInternalIndexServiceBridgeBlueprintLibrary_eventImportUIIHandoffContract_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffContract)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_ContractPath);
	P_GET_PROPERTY_REF(FStrProperty,Z_Param_Out_OutReportPath);
	P_GET_TARRAY_REF(FString,Z_Param_Out_OutWarnings);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UUIIInternalIndexServiceBridgeBlueprintLibrary::ImportUIIHandoffContract(Z_Param_ContractPath,Z_Param_Out_OutReportPath,Z_Param_Out_OutWarnings);
	P_NATIVE_END;
}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary Function ImportUIIHandoffContract

// Begin Class UUIIInternalIndexServiceBridgeBlueprintLibrary
void UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticRegisterNativesUUIIInternalIndexServiceBridgeBlueprintLibrary()
{
	UClass* Class = UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "GetLatestUIIHandoffContractPath", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execGetLatestUIIHandoffContractPath },
		{ "ImportLatestUIIHandoffAndBuildCatalog", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffAndBuildCatalog },
		{ "ImportLatestUIIHandoffBuildCatalogAndEmbeddings", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffBuildCatalogAndEmbeddings },
		{ "ImportLatestUIIHandoffContract", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportLatestUIIHandoffContract },
		{ "ImportUIIHandoffAndBuildCatalog", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffAndBuildCatalog },
		{ "ImportUIIHandoffBuildCatalogAndEmbeddings", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffBuildCatalogAndEmbeddings },
		{ "ImportUIIHandoffContract", &UUIIInternalIndexServiceBridgeBlueprintLibrary::execImportUIIHandoffContract },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(UUIIInternalIndexServiceBridgeBlueprintLibrary);
UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_NoRegister()
{
	return UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticClass();
}
struct Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * Blueprint access to the evidence-only UII -> IIS handoff pipeline.\n *\n * These nodes read local UII handoff contracts and delegate import/catalog/\n * embedding work to IIS public APIs. They do not run UII extraction and do not\n * mutate assets, Blueprints, source files, or project settings.\n */" },
#endif
		{ "IncludePath", "UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
		{ "ModuleRelativePath", "Public/UIIInternalIndexServiceBridgeBlueprintLibrary.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Blueprint access to the evidence-only UII -> IIS handoff pipeline.\n\nThese nodes read local UII handoff contracts and delegate import/catalog/\nembedding work to IIS public APIs. They do not run UII extraction and do not\nmutate assets, Blueprints, source files, or project settings." },
#endif
	};
#endif // WITH_METADATA
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_GetLatestUIIHandoffContractPath, "GetLatestUIIHandoffContractPath" }, // 1319175011
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffAndBuildCatalog, "ImportLatestUIIHandoffAndBuildCatalog" }, // 861700370
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffBuildCatalogAndEmbeddings, "ImportLatestUIIHandoffBuildCatalogAndEmbeddings" }, // 2762024499
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportLatestUIIHandoffContract, "ImportLatestUIIHandoffContract" }, // 3204758206
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffAndBuildCatalog, "ImportUIIHandoffAndBuildCatalog" }, // 1531985190
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffBuildCatalogAndEmbeddings, "ImportUIIHandoffBuildCatalogAndEmbeddings" }, // 467856907
		{ &Z_Construct_UFunction_UUIIInternalIndexServiceBridgeBlueprintLibrary_ImportUIIHandoffContract, "ImportUIIHandoffContract" }, // 2856894555
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UUIIInternalIndexServiceBridgeBlueprintLibrary>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
UObject* (*const Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
	(UObject* (*)())Z_Construct_UPackage__Script_UIIInternalIndexServiceBridge,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::ClassParams = {
	&UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticClass,
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
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::Class_MetaDataParams), Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary()
{
	if (!Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary.OuterSingleton, Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary.OuterSingleton;
}
template<> UIIINTERNALINDEXSERVICEBRIDGE_API UClass* StaticClass<UUIIInternalIndexServiceBridgeBlueprintLibrary>()
{
	return UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticClass();
}
UUIIInternalIndexServiceBridgeBlueprintLibrary::UUIIInternalIndexServiceBridgeBlueprintLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR(UUIIInternalIndexServiceBridgeBlueprintLibrary);
UUIIInternalIndexServiceBridgeBlueprintLibrary::~UUIIInternalIndexServiceBridgeBlueprintLibrary() {}
// End Class UUIIInternalIndexServiceBridgeBlueprintLibrary

// Begin Registration
struct Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridge_Public_UIIInternalIndexServiceBridgeBlueprintLibrary_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, UUIIInternalIndexServiceBridgeBlueprintLibrary::StaticClass, TEXT("UUIIInternalIndexServiceBridgeBlueprintLibrary"), &Z_Registration_Info_UClass_UUIIInternalIndexServiceBridgeBlueprintLibrary, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UUIIInternalIndexServiceBridgeBlueprintLibrary), 935298905U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridge_Public_UIIInternalIndexServiceBridgeBlueprintLibrary_h_1414749001(TEXT("/Script/UIIInternalIndexServiceBridge"),
	Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridge_Public_UIIInternalIndexServiceBridgeBlueprintLibrary_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_TinyToolDevelopment_Git_BridgePlugins_UIIInternalIndexServiceBridge_Source_UIIInternalIndexServiceBridge_Public_UIIInternalIndexServiceBridgeBlueprintLibrary_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
