#pragma once

/*
PCH: No
*/

namespace ba
{
	class FBXLoaderTester : public Application
	{
	public:
		FBXLoaderTester();
		~FBXLoaderTester() override;

	private:
		void Render() override;
		void UpdateDirectX() override;

		bool OnResize() override;

		bool InitDirectX() override;
		void ReleaseDirectX() override;

		void InitLights();

		void SetEffectVariablesPerFrame();
		void SetEffectVariablesPerResize();
		void SetEffectVariablesChangeRarely();

	private:
		void UpdateOnKeyInput() override;

		void OnMouseMove(WPARAM w_par, int x, int y) override;
		void OnMouseLBtnDown(WPARAM w_par, int x, int y) override;
		void OnMouseRBtnDown(WPARAM w_par, int x, int y) override;
		void OnMouseLBtnUp(WPARAM w_par, int x, int y) override;
		void OnMouseRBtnUp(WPARAM w_par, int x, int y) override;

	private:
		Model* model_;
		ModelInstance model_inst_;

		DirectionalLight lights_[3];

		// Properties related to camera contorl.
		//
		static const XMVECTOR kInitCamPos;
		static const XMVECTOR kInitCamTarget;
		static const XMVECTOR kInitCamUp;

		static const float kFovY;
		static const float kNearZ;
		static const float kFarZ;

		static const float kForwardMovingRate;
		static const float kRightMovingRate;
		static const float kUpperMovingRate;
		static const float kRotationRate;

		Camera cam_;

		POINT last_mouse_pos_;
	};
}
