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

		bool Load(const std::string& filename, FBXLoadedModel& out_model);

	private:
		bool LoadNode(FbxNode* node, FBXLoadedModel& out_model);

		bool LoadGeometry(FbxMesh* fbx_mesh, FBXLoadedMesh& out_mesh);
		void LoadTransform(FbxNode* node, XMFLOAT4X4& out_transform);
		void LoadMaterials(FbxNode* node, std::vector<FBXLoadedMaterial>& out_materials);
		void LoadTextureInfos(FbxProperty& prop, FbxTexture::ETextureUse tex_use, std::vector<FBXLoadedTextureInfo>& out_infos);

		void ReadPosition(const FbxVector4& control_point, XMFLOAT3& out_pos);
		bool ReadNormal(const FbxGeometryElementNormal* normal_element, int control_point_idx, int vertex_idx, XMFLOAT3& out_normal);
		bool ReadUV(const FbxGeometryElementUV* uv_element, int control_point_idx, int uv_idx, XMFLOAT2& out_uv);
		bool ReadTangent(const FbxGeometryElementTangent* tangent_element, int control_point_idx, int vertex_idx, XMFLOAT3& out_tangent);
		bool ReadMaterialIndex(const FbxGeometryElementMaterial* material_element, int triangle_idx, int& out_material_idx);

	private:
		FbxManager*		manager_;
		FbxIOSettings*	io_settings_;

		int mesh_counter_;
	};
}
