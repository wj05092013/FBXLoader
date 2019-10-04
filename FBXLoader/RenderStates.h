#pragma once

/*
PCH: Yes
*/

namespace ba
{
	namespace renderstates
	{
		namespace rasterizer
		{
			extern ID3D11RasterizerState* kExample;

			bool Init(ID3D11Device* device);
			void Release();
		}
		
		namespace depthstencil
		{
			bool Init(ID3D11Device* device);
			void Release();
		}

		namespace blend
		{
			bool Init(ID3D11Device* device);
			void Release();
		}

		bool InitAll(ID3D11Device* device);
		void ReleaseAll();
	}
}