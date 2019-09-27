#pragma once

/*
PCH: Yes
*/

namespace ba
{
	namespace inputvertex
	{
		//
		// Different Types Of Vertices
		//

		struct PosNormalTex : public Uncopiable
		{
			struct Vertex
			{
				XMFLOAT3 pos;
				XMFLOAT3 normal;
				XMFLOAT2 uv;
			};
			static const D3D11_INPUT_ELEMENT_DESC kInputElemDesc[3];

			bool Init(ID3D11Device* device);
			void Release();

			ID3D11InputLayout* input_layout = nullptr;
		};

		struct PosNormalTexTangent : public Uncopiable
		{
			struct Vertex
			{
				XMFLOAT3 pos;
				XMFLOAT3 normal;
				XMFLOAT2 uv;
				XMFLOAT3 tangent;
			};
			static const D3D11_INPUT_ELEMENT_DESC kInputElemDesc[4];

			bool Init(ID3D11Device* device);
			void Release();

			ID3D11InputLayout* input_layout = nullptr;
		};

		struct PosNormalTexTanSkinned : public Uncopiable
		{
			struct Vertex
			{
				XMFLOAT3 pos;
				XMFLOAT3 normal;
				XMFLOAT2 uv;
				XMFLOAT3 tangent;
				XMFLOAT3 blend_weights;
				XMBYTE4 bone_indices;
			};
			static const D3D11_INPUT_ELEMENT_DESC kInputElemDesc[6];

			bool Init(ID3D11Device* device);
			void Release();

			ID3D11InputLayout* input_layout = nullptr;
		};


		// Integrated management of all input layouts.
		//
		bool InitAll(ID3D11Device* device);
		void ReleaseAll();

		PosNormalTex kPosNormalTex;
		PosNormalTexTanSkinned kPosNormalTexTanSkinned;
		//__
	}
}