#include "stdafx.h"

bool ba::effects::InitAll(ID3D11Device* device)
{
	if (!kBasicEffect.Init(device, kBasicEffectFileName)) return false;

	return true;
}

void ba::effects::ReleaseAll()
{
	kBasicEffect.Release();
}
