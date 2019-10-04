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
		Model* model_ = nullptr;
		XMFLOAT4X4 world_;
	};
}
