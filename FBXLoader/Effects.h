#pragma once

/*
PCH: Yes
*/

namespace ba
{
	//
	// All 'EffectWrapper' instances are managed in here.
	//

	namespace effects
	{
		bool InitAll(ID3D11Device* device);
		void ReleaseAll();

		// .fxo file names to create effect interfaces with.
		//
		const std::wstring kBasicEffectFileName = L"Basic.fxo";
		//__

		// Instances.
		//
		BasicEffect kBasicEffect;
		//__
	}
}