#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Renderer : public Uncopiable
	{
	public:
		struct RenderingComponents
		{
			ID3D11RenderTargetView* rtv;
			ID3D11DepthStencilView* dsv;
			D3D11_VIEWPORT*			viewport;
			Camera*					cam;
			ShadowMap*				shadow_map;
			SSAOMap*				ssao_map;
		};

		struct EffectVariableBundlePerFrame
		{
			// Empty now.
		};

		struct EffectVariableBundleOnStartAndResize
		{
			// Empty now.
		};

		struct EffectVariableBundleChangeRarely
		{
			light::DirectionalLight*	directional_lights;
			float						fog_start;
			float						fog_range;
			XMVECTOR					fog_color;
			float						shadow_map_size;
			XMMATRIX					to_tex;
		};

	public:
		Renderer();
		~Renderer();

		bool Init(ID3D11Device* device, ID3D11DeviceContext* dc);
		void Release();
			
		void RenderScene(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle);
		void RenderShadowMap(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle);
		void RenderNormalDepthMap(const std::vector<ModelInstance>& model_instances, const EffectVariableBundlePerFrame& bundle);

		void SetEffectVariablesOnStartAndResize(const EffectVariableBundleOnStartAndResize& bundle);

		void SetEffectVariablesChangeRarely(const EffectVariableBundleChangeRarely& bundle);

		void set_rendering_components(RenderingComponents& rendering_component);

	private:
		ID3D11DeviceContext* dc_;
		RenderingComponents rendering_components_;
	};
}
