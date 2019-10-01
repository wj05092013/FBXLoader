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

		bool Load(const std::string& filename, Model& out_model);

	private:
		bool LoadNode(FbxNode* node, Model& out_model);

		bool LoadMesh(FbxMesh* fbx_mesh, Model& out_model);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		bool ReadNormal(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		bool ReadTangent(FbxMesh* fbx_mesh, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);
		bool ReadUV(FbxMesh* fbx_mesh, int control_point_idx, int uv_idx, XMFLOAT2& out_uv);
		
	private:
		FbxManager* manager_;
		FbxIOSettings* io_settings_;
	};
}
