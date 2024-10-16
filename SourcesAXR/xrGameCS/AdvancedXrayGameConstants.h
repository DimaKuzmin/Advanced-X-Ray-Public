﻿#pragma once

namespace GameConstants
{
	void LoadConstants();

	bool GetKnifeSlotEnabled();
	bool GetBinocularSlotEnabled();
	bool GetTorchSlotEnabled();
	bool GetBackpackSlotEnabled();
	bool GetDosimeterSlotEnabled();
	bool GetPantsSlotEnabled();
	bool GetPdaSlotEnabled();
	bool GetTorchHasBattery();
	bool GetArtDetectorUseBattery();
	bool GetAnoDetectorUseBattery();
	bool GetLimitedBolts();
	bool GetActorThirst();
	bool GetActorIntoxication();
	bool GetActorSleepeness();
	bool GetActorAlcoholism();
	bool GetActorNarcotism();
	bool GetArtefactsDegradation();
	bool GetShowWpnInfo();
	bool GetJumpSpeedWeightCalc();
	bool GetHideWeaponInInventory();
	bool GetStopActorIfShoot();
	bool GetReloadIfSprint();
	bool GetColorizeValues();
	bool GetAfRanks();
	bool GetOutfitUseFilters();
	bool GetHideHudOnMaster();
	bool GetActorSkillsEnabled();
	bool GetSleepInfluenceOnPsyHealth();
	bool GetUseHQ_Icons();
	bool GetArtefactPanelEnabled();
	bool GetHUD_UsedItemTextEnabled();
	bool GetLimitedInventory();
	bool GetInventoryItemsAutoVolume();
	bool GetBackpackAnimsEnabled();
	bool GetFoodIrradiation();
	bool GetFoodRotting();
	bool GetOGSE_WpnZoomSystem();
	bool GetQuickThrowGrenadesEnabled();
	bool GetPDA_FlashingIconsEnabled();
	bool GetPDA_FlashingIconsQuestsEnabled();
	bool GetFogInfluenceVolumetricLight();
	int  GetArtefactsCount();
	int  GetIntScriptCMDCount();
	int  GetBOOLScriptCMDCount();
	float GetDistantSndDistance();
	Fvector4 GetRedColor();
	Fvector4 GetGreenColor();
	Fvector4 GetNeutralColor();
	Fvector4 GetSSFX_DefaultDoF();
	Fvector4 GetSSFX_FocusDoF();
	bool GetSSFX_EnableBoreDoF();
	LPCSTR GetAfInfluenceMode();
	LPCSTR GetArtefactDegradationMode();
	LPCSTR GetMoonPhasesMode();
};
