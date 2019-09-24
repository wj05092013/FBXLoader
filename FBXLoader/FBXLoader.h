#pragma once

/*
PCH: Yes
*/

namespace ba
{
	class FBXLoader : public Uncopiable
	{
	public:
		static FBXLoader& GetInstance();

		void Init();
		void Release();

		bool Load(const std::wstring& filename);

	private:
		FbxManager* fbx_mgr_;
		FbxIOSettings* fbx_io_settings_ = nullptr;
	};
}
