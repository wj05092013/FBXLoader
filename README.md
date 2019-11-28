# FBX Loader

![dragon](./ScreenShot/dragon.JPG)

## Explanation

This project was suspended.
Because of lack of information and references to study the FBX SDK(or lack of my search skills), I couldn't continue this project.
Now I'm finding new references.
This project will be resumed when my knowledge about the FBX SDK improves.

## TODO

#### Implement

* Texture blending
* Load materials and textures
* Load animation information

#### Refactor

* Divide the Renderer into multiple ones according to functions.
* Rename the enums. (Enum -> EEnum, kElem -> eElem)

#### Research

* Loading .fbx file
* SRP(Single Responsibility Principle)

## Reference

* FBX SDK Documentation
* Blogs written by who studied FBX SDK before.
* Unreal Engine 4

## Model Files

https://free3d.com/ko/3d-models/fbx

## ERROR

#### Compile-time

	error LNK2001: "public: static char const * const fbxsdk::FbxSurfaceMaterial::sAmbient"(sAmbient@FbxSurfaceMaterial@fbxsdk@@2PBDB) 외부 기호를 확인할 수 없습니다.
    : Use 'libfbxsdk-md.lib' instead of 'libfbxsdk.lib'.

#### Runtime

	In the function ba::Mesh::BuildFaceMaterialIndicesView,
	the 'Width' member of D3D11_TEXTURE1D_DESC must be constrained.
	(D3D10_REQ_TEXTURE1D_U_DIMENSION)
