#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Renderer : public Uncopiable
	{
	public:
		Renderer();
		~Renderer();

		bool Init(ID3D11Device* device, ID3D11DeviceContext* dc);
		void Release();
			
		void RenderNormaly(const std::vector<ModelInstance>& model_instances);
		void RenderShadowMap(const std::vector<ModelInstance>& model_instances, ShadowMap& shadow_map);
		void RenderNormalDepthMap(const std::vector<ModelInstance>& model_instances, SSAOMap& ssao_map);

	private:
		ID3D11DeviceContext* dc_;
	};
}