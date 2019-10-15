#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class Model
	{
	public:
		bool Init(ID3D11Device* device, const FBXLoadedModel& fbx_model);

		std::vector<Mesh> meshes;
	};

	class ModelInstance
	{
	public:
		ModelInstance();

		Model*		model;
		XMMATRIX	scale;
		XMMATRIX	rotation;
		XMMATRIX	translation;
	};
}
