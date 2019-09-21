#pragma once

#include <fbxsdk.h>
#include "util.h"

namespace ba
{
	class FBXLoader : public Uncopiable
	{
	public:
		static FBXLoader& GetInstance();

		void Init();
		void Release();

		util::ErrorCode Load(const std::wstring& filename);

	private:
		FbxManager* fbx_mgr_;
		FbxIOSettings* fbx_io_settings_ = nullptr;
	};
}
