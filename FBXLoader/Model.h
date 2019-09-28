#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Model : public Uncopiable
	{
	public:
		enum EffectType
		{
			kBasic = 0,
			kMax
		};
		
		Model();
	
		void Draw(ID3D11DeviceContext* dc, const XMMATRIX& world);

		EffectType effect_type_;
		bool b_reflect_;

		Mesh mesh_;
		// Add diffuse maps, normal-displacement maps.

	private:
		void DrawWithBasicEffect(ID3D11DeviceContext* dc, const XMMATRIX& world);
	};

	class ModelInstance
	{
	public:
		ModelInstance();

		void Draw(ID3D11DeviceContext* dc);

		Model* model_;
		XMFLOAT4X4 world_;
	};
}
