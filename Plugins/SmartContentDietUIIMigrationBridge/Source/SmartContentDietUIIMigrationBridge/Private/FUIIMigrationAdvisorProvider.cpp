/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet UII Migration Bridge" Unreal Engine plugin.
 * Use of this software is governed by the Fab Standard End User License Agreement
 * (EULA) applicable to this product, available at:
 * https://www.fab.com/eula
 *
 * Except as expressly permitted by the Fab Standard EULA, any reproduction,
 * distribution, modification, or use of this software, in whole or in part,
 * is strictly prohibited.
 *
 * This software is provided on an "AS IS" basis, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied, including but not
 * limited to warranties of merchantability, fitness for a particular purpose,
 * and non-infringement.
 * available at: https://www.fab.com/eula.  */

#include "FUIIMigrationAdvisorProvider.h"

#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "IUnrealIntegrationIntelligenceService.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	FString NormalizePath(const FString& Path)
	{
		if (Path.IsEmpty())
		{
			return FString();
		}

		FString NormalizedPath = FPaths::ConvertRelativePathToFull(Path);
		FPaths::NormalizeFilename(NormalizedPath);
		return NormalizedPath;
	}

	FString UIIOutputRoot()
	{
		return NormalizePath(FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("UnrealIntegrationIntelligence")));
	}

	IUnrealIntegrationIntelligenceService* ResolveUIIService()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		UClass* SubsystemClass = FindObject<UClass>(
			nullptr,
			TEXT("/Script/UnrealIntegrationIntelligenceRuntime.UnrealIntegrationIntelligenceSubsystem"));
		if (!SubsystemClass)
		{
			SubsystemClass = LoadObject<UClass>(
				nullptr,
				TEXT("/Script/UnrealIntegrationIntelligenceRuntime.UnrealIntegrationIntelligenceSubsystem"));
		}
		if (!SubsystemClass)
		{
			return nullptr;
		}

		return Cast<IUnrealIntegrationIntelligenceService>(GEngine->GetEngineSubsystemBase(SubsystemClass));
	}

	bool LoadJsonFile(const FString& Path, TSharedPtr<FJsonObject>& OutObject, FString& OutError)
	{
		FString Json;
		if (!FFileHelper::LoadFileToString(Json, *Path))
		{
			OutError = FString::Printf(TEXT("Unable to read UII handoff JSON: %s"), *Path);
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
		{
			OutError = FString::Printf(TEXT("Unable to parse UII handoff JSON: %s"), *Path);
			return false;
		}

		return true;
	}

	FString GetStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		FString Value;
		return Object.IsValid() && Object->TryGetStringField(FieldName, Value) ? Value : FString();
	}

	bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		bool bValue = false;
		return Object.IsValid() && Object->TryGetBoolField(FieldName, bValue) ? bValue : false;
	}

	TArray<FString> GetStringArrayField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		TArray<FString> Values;
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Object.IsValid() || !Object->TryGetArrayField(FieldName, Array) || !Array)
		{
			return Values;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Array)
		{
			FString StringValue;
			if (Value.IsValid() && Value->TryGetString(StringValue) && !StringValue.IsEmpty())
			{
				Values.Add(StringValue);
			}
		}

		return Values;
	}

	void AddFinding(
		FSmartMigrationResult& OutResult,
		const FString& Subject,
		const FString& Category,
		const FString& Severity,
		const FString& Advice,
		const float EstimatedEffort)
	{
		if (Advice.IsEmpty())
		{
			return;
		}

		FSmartMigrationFinding Finding;
		Finding.Subject = Subject.IsEmpty() ? TEXT("UII handoff") : Subject;
		Finding.Category = Category;
		Finding.Severity = Severity;
		Finding.Advice = Advice;
		Finding.EstimatedEffort = EstimatedEffort;
		OutResult.Findings.Add(Finding);
	}

	bool IsMigrationSensitiveAction(const FString& Action)
	{
		return Action.Contains(TEXT("migration"), ESearchCase::IgnoreCase)
			|| Action.Contains(TEXT("mutate"), ESearchCase::IgnoreCase)
			|| Action.Contains(TEXT("patch"), ESearchCase::IgnoreCase)
			|| Action.Contains(TEXT("copy"), ESearchCase::IgnoreCase)
			|| Action.Contains(TEXT("refactor"), ESearchCase::IgnoreCase)
			|| Action.Contains(TEXT("placement"), ESearchCase::IgnoreCase);
	}

	FString ResolveCandidatePath(const FString& Path, const FString& BaseDirectory)
	{
		if (Path.IsEmpty())
		{
			return FString();
		}

		if (!FPaths::IsRelative(Path))
		{
			return NormalizePath(Path);
		}

		const FString FromBase = NormalizePath(FPaths::Combine(BaseDirectory, Path));
		if (FPaths::FileExists(FromBase))
		{
			return FromBase;
		}

		const FString FromSaved = NormalizePath(FPaths::Combine(FPaths::ProjectSavedDir(), Path));
		if (FPaths::FileExists(FromSaved))
		{
			return FromSaved;
		}

		return NormalizePath(FPaths::Combine(UIIOutputRoot(), Path));
	}

	FString ResolveInputFileRefPath(
		const TSharedPtr<FJsonObject>& ContractObject,
		const FString& LogicalName,
		const FString& BaseDirectory)
	{
		const TArray<TSharedPtr<FJsonValue>>* InputFiles = nullptr;
		if (!ContractObject.IsValid()
			|| !ContractObject->TryGetArrayField(TEXT("input_files"), InputFiles)
			|| !InputFiles)
		{
			return FString();
		}

		for (const TSharedPtr<FJsonValue>& InputFileValue : *InputFiles)
		{
			const TSharedPtr<FJsonObject> InputFile = InputFileValue.IsValid()
				? InputFileValue->AsObject()
				: nullptr;
			if (!InputFile.IsValid() || GetStringField(InputFile, TEXT("logical_name")) != LogicalName)
			{
				continue;
			}

			const FString RelativePath = GetStringField(InputFile, TEXT("relative_path"));
			const FString ResolvedPath = ResolveCandidatePath(RelativePath, BaseDirectory);
			return FPaths::FileExists(ResolvedPath) ? ResolvedPath : FString();
		}

		return FString();
	}

	FString ResolveEnvelopePath(const TSharedPtr<FJsonObject>& ContractObject, const FString& ContractPath)
	{
		const FString BaseDirectory = FPaths::GetPath(ContractPath);
		const FString FlatPath = ResolveCandidatePath(
			GetStringField(ContractObject, TEXT("langgraph_handoff_envelope_path")),
			BaseDirectory);
		if (!FlatPath.IsEmpty() && FPaths::FileExists(FlatPath))
		{
			return FlatPath;
		}

		return ResolveInputFileRefPath(ContractObject, TEXT("langgraph_handoff_envelope"), BaseDirectory);
	}

	void TranslateGuardrails(
		const TSharedPtr<FJsonObject>& Object,
		const FString& Subject,
		FSmartMigrationResult& OutResult)
	{
		for (const FString& Guardrail : GetStringArrayField(Object, TEXT("global_guardrails")))
		{
			AddFinding(
				OutResult,
				Subject,
				TEXT("MigrationGuardrail"),
				TEXT("Info"),
				Guardrail,
				0.0f);
		}

		for (const FString& Guardrail : GetStringArrayField(Object, TEXT("guardrails")))
		{
			AddFinding(
				OutResult,
				Subject,
				TEXT("MigrationGuardrail"),
				TEXT("Info"),
				Guardrail,
				0.0f);
		}

		for (const FString& Action : GetStringArrayField(Object, TEXT("forbidden_actions")))
		{
			if (!IsMigrationSensitiveAction(Action))
			{
				continue;
			}
			AddFinding(
				OutResult,
				Subject,
				TEXT("MigrationGuardrail"),
				TEXT("Medium"),
				FString::Printf(TEXT("UII handoff forbids '%s'. Treat this as evidence-only migration context."), *Action),
				0.0f);
		}
	}

	void TranslateEnvelope(
		const TSharedPtr<FJsonObject>& EnvelopeObject,
		const FString& EnvelopePath,
		const FSmartMigrationRequest& Request,
		FSmartMigrationResult& OutResult)
	{
		const FString HandoffId = GetStringField(EnvelopeObject, TEXT("handoff_id"));
		const FString Subject = HandoffId.IsEmpty() ? EnvelopePath : HandoffId;

		const FString Status = GetStringField(EnvelopeObject, TEXT("status"));
		if (!Status.IsEmpty() && Status != TEXT("Ready"))
		{
			AddFinding(
				OutResult,
				Subject,
				TEXT("HandoffStatus"),
				TEXT("Warning"),
				FString::Printf(TEXT("UII handoff status is '%s'; review warnings before migration planning."), *Status),
				0.25f);
		}

		const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
		if (EnvelopeObject.IsValid()
			&& EnvelopeObject->TryGetArrayField(TEXT("suggested_graph_nodes"), Nodes)
			&& Nodes)
		{
			for (const TSharedPtr<FJsonValue>& NodeValue : *Nodes)
			{
				const TSharedPtr<FJsonObject> Node = NodeValue.IsValid() ? NodeValue->AsObject() : nullptr;
				if (!Node.IsValid())
				{
					continue;
				}

				const FString NodeId = GetStringField(Node, TEXT("node_id"));
				const FString DisplayName = GetStringField(Node, TEXT("display_name"));
				const FString Purpose = GetStringField(Node, TEXT("purpose"));
				FString Advice = Purpose;
				const TArray<FString> ForbiddenOutputs = GetStringArrayField(Node, TEXT("forbidden_outputs"));
				for (const FString& ForbiddenOutput : ForbiddenOutputs)
				{
					if (IsMigrationSensitiveAction(ForbiddenOutput))
					{
						Advice += FString::Printf(
							TEXT(" Forbidden output '%s' confirms SCD must keep this advisory-only."),
							*ForbiddenOutput);
						break;
					}
				}

				AddFinding(
					OutResult,
					NodeId.IsEmpty() ? DisplayName : NodeId,
					TEXT("MigrationReviewStep"),
					TEXT("Info"),
					Advice,
					0.5f);
			}
		}

		TranslateGuardrails(EnvelopeObject, Subject, OutResult);

		const FString Target = Request.TargetEngineVersion.IsEmpty()
			? TEXT("the requested target")
			: Request.TargetEngineVersion;
		OutResult.Rationale = FString::Printf(
			TEXT("Translated UII LangGraph handoff evidence into advisory SCD migration findings for %s. SCD does not execute migration decisions."),
			*Target);
	}

	bool TranslateContract(
		const TSharedPtr<FJsonObject>& ContractObject,
		const FString& ContractPath,
		const FSmartMigrationRequest& Request,
		FSmartMigrationResult& OutResult)
	{
		const FString EnvelopePath = ResolveEnvelopePath(ContractObject, ContractPath);
		if (!EnvelopePath.IsEmpty())
		{
			TSharedPtr<FJsonObject> EnvelopeObject;
			FString LoadError;
			if (LoadJsonFile(EnvelopePath, EnvelopeObject, LoadError))
			{
				TranslateEnvelope(EnvelopeObject, EnvelopePath, Request, OutResult);
				return true;
			}
		}

		TranslateGuardrails(ContractObject, ContractPath, OutResult);

		if (ContractObject.IsValid()
			&& ContractObject->HasField(TEXT("allows_migration_decision"))
			&& !GetBoolField(ContractObject, TEXT("allows_migration_decision")))
		{
			AddFinding(
				OutResult,
				ContractPath,
				TEXT("MigrationGuardrail"),
				TEXT("Medium"),
				TEXT("The UII handoff explicitly does not allow migration decisions; use it only as evidence for human-reviewed advice."),
				0.0f);
		}

		if (OutResult.Findings.Num() == 0)
		{
			AddFinding(
				OutResult,
				ContractPath,
				TEXT("UIIHandoff"),
				TEXT("Info"),
				TEXT("A UII handoff is available, but it did not contain migration-specific findings. Review the evidence package before planning upgrades."),
				0.25f);
		}

		OutResult.Rationale = TEXT("Translated UII handoff contract guardrails into advisory SCD migration findings.");
		return true;
	}
}

bool FUIIMigrationAdvisorProvider::ComputeMigrationAdvice(
	const FSmartMigrationRequest& Request,
	FSmartMigrationResult& OutResult)
{
	OutResult = FSmartMigrationResult();
	OutResult.bApproximate = true;

	IUnrealIntegrationIntelligenceService* Service = ResolveUIIService();
	if (!Service)
	{
		OutResult.Rationale = TEXT("Unreal Integration Intelligence service is not available.");
		return false;
	}

	if (!Service->HasMigrationHandoff())
	{
		OutResult.Rationale = TEXT("No UII migration handoff is available. Run UII evidence export before requesting migration advice.");
		return true;
	}

	const FString HandoffPath = Service->GetLatestMigrationHandoffPath();
	if (HandoffPath.IsEmpty() || !FPaths::FileExists(HandoffPath))
	{
		OutResult.Rationale = FString::Printf(TEXT("UII reported a handoff, but the file is missing: %s"), *HandoffPath);
		return false;
	}

	TSharedPtr<FJsonObject> HandoffObject;
	FString LoadError;
	if (!LoadJsonFile(HandoffPath, HandoffObject, LoadError))
	{
		OutResult.Rationale = LoadError;
		return false;
	}

	if (HandoffObject->HasField(TEXT("suggested_graph_nodes")))
	{
		TranslateEnvelope(HandoffObject, HandoffPath, Request, OutResult);
	}
	else
	{
		TranslateContract(HandoffObject, HandoffPath, Request, OutResult);
	}

	return true;
}

bool FUIIMigrationAdvisorProvider::IsImplementationAvailable() const
{
	return ResolveUIIService() != nullptr;
}
