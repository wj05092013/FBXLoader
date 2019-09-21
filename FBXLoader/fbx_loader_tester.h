#pragma once

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
	};
}
