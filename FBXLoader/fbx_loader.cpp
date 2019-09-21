#include "stdafx.h"

ba::FBXLoader& ba::FBXLoader::GetInstance()
{
	static FBXLoader instance;
	return instance;
}

void ba::FBXLoader::Init()
{
	if (fbx_mgr_ == nullptr)
	{
		fbx_mgr_ = FbxManager::Create();
		fbx_io_settings_ = FbxIOSettings::Create(fbx_mgr_, IOSROOT);
	}
}

void ba::FBXLoader::Release()
{
	fbx_mgr_->Destroy();
	fbx_mgr_ = nullptr;

	fbx_io_settings_->Destroy();
	fbx_io_settings_ = nullptr;
}
