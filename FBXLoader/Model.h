#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Model
	{
	public:
		Model();

		bool Init(std::wstring file_name);
		void Release();

		void Draw();

	private:
		Mesh<inputvertex::PosNormalTexTangent::Vertex> mesh_;

		// Add diffuse maps, normal-displacement maps.
	};

	class ModelInstance
	{
	public:
		ModelInstance(Model* model);

		void Draw();

		void set_world(const XMFLOAT4X4& world);

	private:
		Model* model_;
		XMFLOAT4X4 world_;
	};
}
