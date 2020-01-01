#pragma once

#include "pch.h"

namespace pong
{
	class RenderingCtx final
	{
	public:
		// the amount of buffers used within the rendering.
		static const int cBufferCount = 2;

		RenderingCtx();

		void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);

		void SetAlpha(float alpha) { mAlpha = alpha; }

		float GetAlpha() const { return mAlpha; }
	private:
		void D3DInit();
		void D3DCreateDevice();
	private:
		winrt::com_ptr<IDXGIFactory4>	mDXGIFactory;
		winrt::com_ptr<IDXGIAdapter1>	mDXGIAdapter;

		winrt::com_ptr<ID3D12Device>				mDevice;
		winrt::com_ptr<ID3D12CommandQueue>			mCommandQueue;
		winrt::com_ptr<ID3D12DescriptorHeap>		mRTVHeap;
		winrt::com_ptr<ID3D12CommandAllocator>		mCommandAllocators[cBufferCount];
		winrt::com_ptr<ID3D12RootSignature>			mRootSignature;
		winrt::com_ptr<ID3DBlob>					mVertexShader;
		winrt::com_ptr<ID3DBlob>					mPixelShader;
		winrt::com_ptr<ID3D12PipelineState>			mPipelineState;
		winrt::com_ptr<ID3D12GraphicsCommandList>	mCommandList;

		UINT	mRTVDescriptorSize;

		winrt::com_ptr<ID3D12Fence> mFence;
		UINT64						mFenceValues[cBufferCount];
		HANDLE						mFenceEvent;

		float mAlpha;
	};
}
