#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Model : public Uncopiable
	{
	public:
		Mesh mesh_;
		// Add diffuse maps, normal-displacement maps.
	};

	class ModelInstance
	{
	public:
		ModelInstance();

		Model* model_;
		XMFLOAT4X4 world_;
	};
}
