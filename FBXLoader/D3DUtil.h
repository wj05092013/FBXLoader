#pragma once

/*
PCH: Yes
*/

namespace ba
{
	template<class DataType>
	bool CreateTexRTVAndSRV(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, const std::vector<DataType>& init_data, ID3D11RenderTargetView** out_rtv, ID3D11ShaderResourceView** out_srv);
}