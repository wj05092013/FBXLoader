#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Model
	{
	public:
		bool Init(ID3D11Device* device, const FBXLoader::FBXLoaderModel& fbx_model);

		std::vector<Mesh> meshes;

		// Add diffuse maps, normal-displacement maps.
	};

	class ModelInstance
	{
	public:
		ModelInstance();

		Model* model;

		XMMATRIX scale;
		XMMATRIX rotation;
		XMMATRIX translation;
	};
}
