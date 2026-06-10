/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet IIS Similarity Bridge" Unreal Engine plugin.
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

#include "SCDHealthEvidenceIISBridgeCommon.h"

#include "Engine/Engine.h"
#include "IISSubsystem.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "UObject/UObjectGlobals.h"

IInternalIndexService* SCDHealthEvidenceIISBridgeCommon::ResolveIIS()
{
	FModuleManager::Get().LoadModulePtr<IModuleInterface>(TEXT("InternalIndexService"));
	if (!GEngine)
	{
		return nullptr;
	}

	UClass* SubsystemClass = FindObject<UClass>(nullptr, TEXT("/Script/InternalIndexService.IISSubsystem"));
	if (!SubsystemClass)
	{
		SubsystemClass = LoadObject<UClass>(nullptr, TEXT("/Script/InternalIndexService.IISSubsystem"));
	}
	if (!SubsystemClass || !SubsystemClass->IsChildOf(UEngineSubsystem::StaticClass()))
	{
		return nullptr;
	}

	return Cast<UIISSubsystem>(GEngine->GetEngineSubsystemBase(SubsystemClass));
}
