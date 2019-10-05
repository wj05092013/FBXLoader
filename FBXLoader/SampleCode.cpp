#include <fbxsdk.h>

BOOL FbxLoader::_loadFbxMaterials()
{
	Int32 numberOfMaterials = mFbxScene->GetMaterialCount();

	for (Int32 m = 0; m < numberOfMaterials; ++m)
	{
		KFbxSurfaceMaterial* fbxMaterial = mFbxScene->GetMaterial(m);

		KFbxProperty prop;
		_FBX_MATERIAL_INFO* info = new _FBX_MATERIAL_INFO;
		info->materialIndex = m;

		//----- if it phong shading.

		if (fbxMaterial->GetClassId().Is(KFbxSurfacePhong::ClassId))
		{
			KFbxSurfacePhong* phongMaterial = dynamic_cast<KFbxSurfacePhong*>(fbxMaterial);

			info->emissiveColor = _toD3DXVECTOR3(phongMaterial->GetEmissiveColor());
			info->emissiveFactor = _toSingle(phongMaterial->GetEmissiveFactor());
			info->ambientColor = _toD3DXVECTOR3(phongMaterial->GetAmbientColor());
			info->ambientFactor = _toSingle(phongMaterial->GetAmbientFactor());
			info->diffuseColor = _toD3DXVECTOR3(phongMaterial->GetDiffuseColor());
			info->diffuseFactor = _toSingle(phongMaterial->GetDiffuseFactor());
			info->bump = _toD3DXVECTOR3(phongMaterial->GetBump());
			info->bumpFactor = _toSingle(phongMaterial->GetBumpFactor());
			info->transparentColor = _toD3DXVECTOR3(phongMaterial->GetTransparentColor());
			info->transparencyFactor = _toSingle(phongMaterial->GetTransparencyFactor());
			info->displacementColor = _toD3DXVECTOR3(phongMaterial->GetDisplacementColor());
			info->displacementFactor = _toSingle(phongMaterial->GetDisplacementFactor());
		}
		else
		{

		}

		//----- load texture file path.

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sEmissive);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sDiffuse);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sSpecular);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sSpecularFactor);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sShininess);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sBump);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sBumpFactor);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sTransparentColor);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sReflection);
		_loadFbxTexturePropety(info, prop);

		prop = fbxMaterial->FindProperty(KFbxSurfaceMaterial::sDisplacementColor);
		_loadFbxTexturePropety(info, prop);

		//----- add material info.

		info->name = fbxMaterial->GetName();

		mMaterialInfoArray.push_back(info);
	}

	return TRUE;
}

[��ó] FBX file ���� Mesh loading �ϱ� - 3. ����(material) | �ۼ��� ����Ǫ