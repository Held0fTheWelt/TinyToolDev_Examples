/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "Internal Index Service" Unreal Engine plugin.
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

#include "InternalIndexServiceModule.h"

#include "IISLocalIndexService.h"
#include "IISStoragePaths.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogInternalIndexService, Log, All);

FInternalIndexServiceModule::FInternalIndexServiceModule() = default;

FInternalIndexServiceModule::~FInternalIndexServiceModule() = default;

void FInternalIndexServiceModule::StartupModule()
{
	UE_LOG(LogInternalIndexService, Log, TEXT("InternalIndexService started."));

	TArray<FString> Warnings;
	FIISStoragePaths::EnsureDefaultFolders(nullptr, &Warnings);
	for (const FString& Warning : Warnings)
	{
		UE_LOG(LogInternalIndexService, Warning, TEXT("%s"), *Warning);
	}

	LocalService = MakeUnique<FIISLocalIndexService>();
}

void FInternalIndexServiceModule::ShutdownModule()
{
	UE_LOG(LogInternalIndexService, Log, TEXT("InternalIndexService shut down."));
	LocalService.Reset();
}

FInternalIndexServiceModule& FInternalIndexServiceModule::Get()
{
	return FModuleManager::LoadModuleChecked<FInternalIndexServiceModule>(TEXT("InternalIndexService"));
}

bool FInternalIndexServiceModule::IsAvailable()
{
	return FModuleManager::Get().IsModuleLoaded(TEXT("InternalIndexService"));
}

IInternalIndexService& FInternalIndexServiceModule::GetService()
{
	check(LocalService.IsValid());
	return *LocalService;
}

const IInternalIndexService& FInternalIndexServiceModule::GetService() const
{
	check(LocalService.IsValid());
	return *LocalService;
}

IMPLEMENT_MODULE(FInternalIndexServiceModule, InternalIndexService)
