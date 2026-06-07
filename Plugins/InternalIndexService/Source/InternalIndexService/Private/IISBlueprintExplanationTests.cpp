/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"
#include "IISBlueprintExplanationService.h"

namespace IISBlueprintExplanationTest
{
	static const TCHAR* RealUiiIRFixture = TEXT(R"({
  "schema_version": "0.1.0",
  "asset_path": "/Game/Blueprints/BP_Door.BP_Door",
  "blueprint_name": "BP_Door",
  "parent_class": "AActor",
  "unsupported_node_classes": ["K2Node_CustomEvent_Legacy"],
  "variables": [{"name": "bIsOpen", "type_sub_category_object": "/Game/Meshes/SM_Door.SM_Door"}],
  "components": [{"name": "DoorMesh", "component_class": "UStaticMeshComponent"}],
  "graphs": [{
    "graph_name": "EventGraph",
    "graph_kind": "EventGraph",
    "nodes": [
      {"event_name": "ReceiveBeginPlay", "supported_specialization": true},
      {"function_name": "OpenDoor", "function_owner": "BP_Door", "supported_specialization": true},
      {"node_class": "K2Node_Unsupported", "supported_specialization": false},
      {"function_name": "ServerOpenDoor", "function_owner": "BP_Door", "supported_specialization": true}
    ]
  }]
})");

	static bool WriteTempBlueprintIR(const FString& BlueprintsDir, const FString& FileStem, const FString& JsonBody, FString& OutFilePath)
	{
		IFileManager::Get().MakeDirectory(*BlueprintsDir, true);
		OutFilePath = BlueprintsDir / FString::Printf(TEXT("%s.blueprint_ir.json"), *FileStem);
		return FFileHelper::SaveStringToFile(JsonBody, *OutFilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISBlueprintAssemblyFromIR,
	"InternalIndexService.Blueprint.AssemblyFromIR",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISBlueprintAssemblyFromIR::RunTest(const FString& Parameters)
{
	FIISBlueprintExplanation Out;
	const bool bOk = FIISBlueprintExplanationService::AssembleFromIRJson(
		IISBlueprintExplanationTest::RealUiiIRFixture,
		TEXT("/tmp/BP_Door.blueprint_ir.json"),
		Out);

	TestTrue(TEXT("ok"), bOk);
	TestTrue(TEXT("ir available"), Out.bIRAvailable);
	TestEqual(TEXT("name"), Out.BlueprintName, FString(TEXT("BP_Door")));
	TestEqual(TEXT("parent"), Out.ParentClass, FString(TEXT("AActor")));
	TestEqual(TEXT("graphs"), Out.Graphs.Num(), 1);
	TestEqual(TEXT("graph name"), Out.Graphs[0].GraphName, FString(TEXT("EventGraph")));
	TestEqual(TEXT("node count"), Out.Graphs[0].NodeCount, 4);
	TestTrue(TEXT("variables"), Out.Variables.Contains(TEXT("bIsOpen")));
	TestTrue(TEXT("components"), Out.Components.Contains(TEXT("DoorMesh")));
	TestTrue(TEXT("unsupported class"), Out.UnsupportedNodes.Contains(TEXT("K2Node_CustomEvent_Legacy")));
	TestTrue(TEXT("unsupported node"), Out.UnsupportedNodes.Contains(TEXT("K2Node_Unsupported")));
	TestTrue(TEXT("referenced asset"), Out.ReferencedAssets.Contains(TEXT("/Game/Meshes/SM_Door.SM_Door")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISBlueprintUnsupportedFromIR,
	"InternalIndexService.Blueprint.UnsupportedFromIR",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISBlueprintUnsupportedFromIR::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
  "blueprint_name": "BP_Test",
  "graphs": [{"graph_name": "G", "graph_kind": "FunctionGraph", "nodes": [
    {"node_class": "K2Node_Opaque", "supported_specialization": false}
  ]}]
})");
	FIISBlueprintExplanation Out;
	TestTrue(TEXT("assemble"), FIISBlueprintExplanationService::AssembleFromIRJson(Json, TEXT(""), Out));
	TestTrue(TEXT("unsupported node listed"), Out.UnsupportedNodes.Contains(TEXT("K2Node_Opaque")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISBlueprintNetworkHints,
	"InternalIndexService.Blueprint.NetworkHints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISBlueprintNetworkHints::RunTest(const FString& Parameters)
{
	const FString Json = TEXT(R"({
  "blueprint_name": "BP_Net",
  "graphs": [{"graph_name": "G", "graph_kind": "EventGraph", "nodes": [
    {"function_name": "ServerApplyDamage", "function_owner": "AActor"}
  ]}]
})");
	FIISBlueprintExplanation Out;
	TestTrue(TEXT("assemble"), FIISBlueprintExplanationService::AssembleFromIRJson(Json, TEXT(""), Out));
	TestTrue(TEXT("network hint present"), Out.NetworkAuthorityHints.Num() >= 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISExplainBlueprintUsesIRWhenAvailable,
	"InternalIndexService.Blueprint.ExplainUsesIR",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISExplainBlueprintUsesIRWhenAvailable::RunTest(const FString& Parameters)
{
	const FString BlueprintsDir = FPaths::ProjectIntermediateDir() / TEXT("IISBlueprintExplainTests") / TEXT("blueprints");

	FString IrPath;
	TestTrue(
		TEXT("write temp IR"),
		IISBlueprintExplanationTest::WriteTempBlueprintIR(
			BlueprintsDir,
			TEXT("BP_Door"),
			IISBlueprintExplanationTest::RealUiiIRFixture,
			IrPath));

	FIISAgentToolRequest Request;
	Request.AssetPath = TEXT("/Game/Blueprints/BP_Door.BP_Door");
	Request.MaxResults = 5;

	FIISAgentToolResponse Response;
	const bool bOk = FIISAgentAccessService::ExplainBlueprint(Request, Response);

	TestTrue(TEXT("explain ok"), bOk);
	TestTrue(TEXT("ir available"), Response.BlueprintExplanation.bIRAvailable);
	TestEqual(TEXT("blueprint name"), Response.BlueprintExplanation.BlueprintName, FString(TEXT("BP_Door")));
	TestTrue(TEXT("graphs populated"), Response.BlueprintExplanation.Graphs.Num() > 0);

	bool bHasRetrievedOnlyWarning = false;
	for (const FString& Warning : Response.Warnings)
	{
		if (Warning.Contains(TEXT("retrieved Blueprint evidence, not a generated explanation")))
		{
			bHasRetrievedOnlyWarning = true;
			break;
		}
	}
	TestFalse(TEXT("no fallback-only warning"), bHasRetrievedOnlyWarning);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISExplainBlueprintFallback,
	"InternalIndexService.Blueprint.ExplainFallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISExplainBlueprintFallback::RunTest(const FString& Parameters)
{
	FIISAgentToolRequest Request;
	Request.AssetPath = TEXT("/Game/Definitely/MissingBlueprint_ZZZ_999");
	Request.MaxResults = 3;

	FIISAgentToolResponse Response;
	const bool bOk = FIISAgentAccessService::ExplainBlueprint(Request, Response);

	TestFalse(TEXT("ir not available"), Response.BlueprintExplanation.bIRAvailable);
	bool bHasRetrievedWarning = false;
	for (const FString& Warning : Response.Warnings)
	{
		if (Warning.Contains(TEXT("retrieved Blueprint evidence, not a generated explanation")))
		{
			bHasRetrievedWarning = true;
			break;
		}
	}
	TestTrue(TEXT("fallback warning"), bHasRetrievedWarning);
	return true;
}

#endif
