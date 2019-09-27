#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class FBXLoader : public Uncopiable
	{
	private:
		FBXLoader();

	public:
		static FBXLoader& GetInstance();

		void Init();
		void Release();

		bool Load(const std::string& filename);

	private:
		void Load(FbxNode* node);

		void LoadMesh(FbxMesh* mesh);
		void LoadVertices(FbxMesh* mesh, std::vector<inputvertex::PosNormalTex::Vertex>& out_vertices);
		void LoadIndices(FbxMesh* mesh, std::vector<UINT>& out_indices);



	private:
		FbxManager* manager_;
		FbxIOSettings* io_settings_;
	};
}
