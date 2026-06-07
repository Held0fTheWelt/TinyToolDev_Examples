/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "IISAgentAccessService.h"
#include "IISUsageGraphImporter.h"
#include "IISAgentAccessTypes.h"

namespace IISUsageGraphTest
{
	static const TCHAR* NestedCppSymbolFixture = TEXT(R"({
  "files": [
    {
      "relative_path": "Source/Demo/DemoActor.h",
      "module_name": "Demo",
      "symbols": [
        {
          "name": "TargetSymbol",
          "qualified_name": "ADemoActor::TargetSymbol",
          "symbol_kind": "function",
          "line_number": 42
        }
      ]
    }
  ]
})");

	static bool BuildCategorizeFixture(TArray<FString>& OutWarnings)
	{
		FIISSymbolRecord Symbol;
		Symbol.SymbolId = FIISUsageGraphImporter::DeriveSymbolId(TEXT("ADemoActor::TargetSymbol"), TEXT("TargetSymbol"), TEXT("Demo"), 42);
		Symbol.Name = TEXT("TargetSymbol");
		Symbol.QualifiedName = TEXT("ADemoActor::TargetSymbol");
		Symbol.Kind = TEXT("function");
		Symbol.ModuleName = TEXT("Demo");

		TArray<FIISSymbolRecord> Symbols;
		Symbols.Add(Symbol);

		TArray<FIISUsageRecord> Usages;
		FIISUsageRecord Decl;
		Decl.SymbolId = Symbol.SymbolId;
		Decl.UsageKind = EIISUsageKind::Declaration;
		Decl.Location.RelativePath = TEXT("Source/Demo/DemoActor.h");
		Usages.Add(Decl);

		FIISUsageRecord RefA;
		RefA.SymbolId = Symbol.SymbolId;
		RefA.UsageKind = EIISUsageKind::Reference;
		RefA.Location.RelativePath = TEXT("Source/Demo/DemoActor.cpp");
		Usages.Add(RefA);

		FIISUsageRecord RefB;
		RefB.SymbolId = Symbol.SymbolId;
		RefB.UsageKind = EIISUsageKind::Reference;
		RefB.Location.RelativePath = TEXT("Source/Demo/DemoUtil.cpp");
		Usages.Add(RefB);

		TArray<FIISCallEdge> CallEdges;
		FIISCallEdge Call;
		Call.CallerSymbolId = Symbol.SymbolId;
		Call.CalleeSymbolId = FIISUsageGraphImporter::DeriveSymbolId(TEXT("Other::Callee"), TEXT("Callee"), TEXT("Demo"), 10);
		Call.Evidence.RelativePath = TEXT("Source/Demo/DemoActor.cpp");
		CallEdges.Add(Call);

		TArray<FIISAssetReference> AssetRefs;
		FIISAssetReference AssetRef;
		AssetRef.AssetPath = TEXT("/Game/Maps/DemoMap.DemoMap");
		AssetRef.ReferencingSymbolId = Symbol.SymbolId;
		AssetRef.RefKind = TEXT("referencer");
		AssetRefs.Add(AssetRef);

		TArray<FIISBlueprintReference> BlueprintRefs;
		return FIISUsageGraphImporter::ImportGraphPayloadForTest(Symbols, Usages, CallEdges, AssetRefs, BlueprintRefs, OutWarnings);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphParseSymbolExport,
	"InternalIndexService.UsageGraph.ParseSymbolExport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphParseSymbolExport::RunTest(const FString& Parameters)
{
	TArray<FIISSymbolRecord> Symbols;
	TArray<FIISUsageRecord> Usages;
	TArray<FString> Warnings;
	const bool bParsed = FIISUsageGraphImporter::ParseCppSymbolIndexJson(
		IISUsageGraphTest::NestedCppSymbolFixture,
		Symbols,
		Usages,
		Warnings);

	TestTrue(TEXT("parse succeeds"), bParsed);
	TestEqual(TEXT("one symbol"), Symbols.Num(), 1);
	TestEqual(TEXT("symbol name"), Symbols[0].Name, FString(TEXT("TargetSymbol")));
	TestEqual(TEXT("qualified name"), Symbols[0].QualifiedName, FString(TEXT("ADemoActor::TargetSymbol")));
	TestEqual(TEXT("one declaration usage"), Usages.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphQueryCategorizes,
	"InternalIndexService.UsageGraph.QueryCategorizes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphQueryCategorizes::RunTest(const FString& Parameters)
{
	TArray<FString> Warnings;
	TestTrue(TEXT("fixture import"), IISUsageGraphTest::BuildCategorizeFixture(Warnings));

	FIISUsageQueryResult Result;
	TestTrue(TEXT("query succeeds"), FIISUsageGraphImporter::QueryUsages(TEXT("TargetSymbol"), Result));
	TestTrue(TEXT("graph evidence available"), Result.bGraphEvidenceAvailable);
	TestEqual(TEXT("one declaration"), Result.Declarations.Num(), 1);
	TestEqual(TEXT("two references"), Result.References.Num(), 2);
	TestEqual(TEXT("one call edge"), Result.Calls.Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphFallbackLexical,
	"InternalIndexService.UsageGraph.FallbackLexical",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphFallbackLexical::RunTest(const FString& Parameters)
{
	FIISAgentToolRequest Request;
	Request.SymbolName = TEXT("DefinitelyMissingSymbolForGraphFallback");
	Request.MaxResults = 3;

	FIISAgentToolResponse Response;
	const bool bOk = FIISAgentAccessService::FindUsages(Request, Response);

	TestTrue(TEXT("find usages returns"), bOk || Response.Status == EIISAgentToolStatus::Empty);
	TestFalse(TEXT("no graph evidence"), Response.UsageGraph.bGraphEvidenceAvailable);
	bool bHasLexicalWarning = false;
	for (const FString& Warning : Response.Warnings)
	{
		if (Warning.Contains(TEXT("lexical evidence search")))
		{
			bHasLexicalWarning = true;
			break;
		}
	}
	TestTrue(TEXT("lexical warning present"), bHasLexicalWarning);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphBlueprintOrAssetRef,
	"InternalIndexService.UsageGraph.BlueprintOrAssetRef",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphBlueprintOrAssetRef::RunTest(const FString& Parameters)
{
	TArray<FString> Warnings;
	TestTrue(TEXT("fixture import"), IISUsageGraphTest::BuildCategorizeFixture(Warnings));

	FIISUsageQueryResult Result;
	TestTrue(TEXT("asset query"), FIISUsageGraphImporter::QueryUsages(TEXT("DemoMap"), Result));
	TestTrue(TEXT("asset ref present"), Result.AssetRefs.Num() >= 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphResolveEvidencePaths,
	"InternalIndexService.UsageGraph.ResolveEvidencePaths",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphResolveEvidencePaths::RunTest(const FString& Parameters)
{
	const FString Fixture = TEXT(R"({"rag_export_package_path":"pkg/rag_export_package.json"})");
	const FString TempDir = FPaths::ProjectIntermediateDir() / TEXT("IISUsageGraphTests");
	IFileManager::Get().MakeDirectory(*TempDir, true);
	const FString ContractPath = TempDir / TEXT("iis_import_contract.json");
	FFileHelper::SaveStringToFile(Fixture, *ContractPath);

	const FString PkgDir = TempDir / TEXT("pkg");
	IFileManager::Get().MakeDirectory(*PkgDir, true);
	const FString RagPackage = FString::Printf(
		TEXT(R"({"source_references":[{"relative_path":"../evidence/cpp/cpp_symbol_index.json"}]})"));
	FFileHelper::SaveStringToFile(RagPackage, *(PkgDir / TEXT("rag_export_package.json")));

	const FString EvidenceRoot = TempDir / TEXT("evidence");
	const FString CppDir = EvidenceRoot / TEXT("cpp");
	IFileManager::Get().MakeDirectory(*CppDir, true);
	FFileHelper::SaveStringToFile(TEXT("{}"), *(CppDir / TEXT("cpp_symbol_index.json")));

	FResolvedUsageEvidencePaths Paths;
	TArray<FString> Warnings;
	const bool bResolved = FIISUsageGraphImporter::ResolveEvidencePathsFromHandoffContract(ContractPath, Paths, Warnings);
	TestTrue(TEXT("resolve succeeds"), bResolved);
	TestTrue(TEXT("cpp index path set"), Paths.CppSymbolIndexPath.EndsWith(TEXT("cpp_symbol_index.json")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUsageGraphTombstonedSymbolExcluded,
	"InternalIndexService.UsageGraph.TombstonedSymbolExcluded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUsageGraphTombstonedSymbolExcluded::RunTest(const FString& Parameters)
{
	FIISSymbolRecord Symbol;
	Symbol.SymbolId = TEXT("sym.tombstone.test");
	Symbol.Name = TEXT("TombstoneSymbol");
	Symbol.QualifiedName = TEXT("ATombstone::TombstoneSymbol");
	Symbol.Kind = TEXT("function");
	Symbol.ModuleName = TEXT("Demo");
	Symbol.LifecycleState = TEXT("tombstoned");

	TArray<FIISSymbolRecord> Symbols;
	Symbols.Add(Symbol);

	TArray<FIISUsageRecord> Usages;
	FIISUsageRecord Decl;
	Decl.SymbolId = Symbol.SymbolId;
	Decl.UsageKind = EIISUsageKind::Declaration;
	Decl.Location.RelativePath = TEXT("Source/Demo/Tombstone.h");
	Usages.Add(Decl);

	TArray<FIISCallEdge> CallEdges;
	TArray<FIISAssetReference> AssetRefs;
	TArray<FIISBlueprintReference> BlueprintRefs;
	TArray<FString> Warnings;
	TestTrue(TEXT("import tombstoned symbol"),
		FIISUsageGraphImporter::ImportGraphPayloadForTest(Symbols, Usages, CallEdges, AssetRefs, BlueprintRefs, Warnings));

	FIISUsageQueryResult Result;
	TestTrue(TEXT("query succeeds"), FIISUsageGraphImporter::QueryUsages(TEXT("TombstoneSymbol"), Result));
	TestEqual(TEXT("no declarations"), Result.Declarations.Num(), 0);
	TestEqual(TEXT("no references"), Result.References.Num(), 0);
	TestFalse(TEXT("no graph evidence"), Result.bGraphEvidenceAvailable);
	return true;
}

#endif
