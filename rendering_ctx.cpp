#include "pch.h"

using namespace pong;
using namespace winrt;
using namespace winrt::Windows::UI::Core;

// a helper macro to allow writing shader code as a multiline string.
#define SHADER(CODE) #CODE

// a shortcut macro to keep code more readable for uuid void ptr assignments.
#define UUID_PPV(x) __uuidof(x), x.put_void()

RenderingCtx::RenderingCtx() : mRTVDescriptorSize(0), mFenceValues{}, mAlpha(0.f)
{
	// inititlize Direct3D.
	D3DInit();

	// create a command queue for the rendering operations.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	check_hresult(mDevice->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), mCommandQueue.put_void()));
	mCommandQueue->SetName(L"mCommandQueue");

	// create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = cBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	check_hresult(mDevice->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), mRTVHeap.put_void()));
	mRTVHeap->SetName(L"mRTVHeap");

	// store the size of a render target descriptor for later use.
	mRTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// create a command allocator for each buffer.
	for (auto i = 0u; i < cBufferCount; i++) {
		check_hresult(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), mCommandAllocators[i].put_void()));
	}

	// create a root signature for resources.
	com_ptr<ID3DBlob> signature, error;
	D3D12_ROOT_SIGNATURE_DESC signatureDesc = {};
	signatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	check_hresult(D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.put(), error.put()));
	check_hresult(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), __uuidof(ID3D12RootSignature), mRootSignature.put_void()));

	// create and compile vertex and pixel shader.
	auto shaderSrc = SHADER(
		struct PSInput
		{
			float4 position : SV_POSITION;
			float4 color : COLOR;
		};

		PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
		{
			PSInput result;
			result.position = position;
			result.color = color;
			return result;
		}

		float4 PSMain(PSInput input) : SV_TARGET
		{
		  return input.color;
		}
	);
	check_hresult(D3DCompile(shaderSrc, strlen(shaderSrc), "", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, mVertexShader.put(), error.put()));
	check_hresult(D3DCompile(shaderSrc, strlen(shaderSrc), "", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, mPixelShader.put(), error.put()));

	// define the layout for the input vertex data.
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputDescriptor = {
	  {
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0
	  },
	  {
		"COLOR",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		12,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA
	  }
	};

	// create a descriptor for the rasterizer state (derived from CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT))
	D3D12_RASTERIZER_DESC rasterizerDescriptor = {};
	rasterizerDescriptor.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDescriptor.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDescriptor.FrontCounterClockwise = false;
	rasterizerDescriptor.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDescriptor.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDescriptor.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDescriptor.DepthClipEnable = true;
	rasterizerDescriptor.MultisampleEnable = false;
	rasterizerDescriptor.AntialiasedLineEnable = false;
	rasterizerDescriptor.ForcedSampleCount = 0;
	rasterizerDescriptor.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// create a descriptor for the blend state (derived from CD3DX12_BLEND_DESC(CD3DX12_DEFAULT))
	D3D12_BLEND_DESC blendDescriptor = {};
	blendDescriptor.AlphaToCoverageEnable = false;
	blendDescriptor.IndependentBlendEnable = false;
	blendDescriptor.RenderTarget[0].BlendEnable = false;
	blendDescriptor.RenderTarget[0].LogicOpEnable = false;
	blendDescriptor.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDescriptor.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDescriptor.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDescriptor.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDescriptor.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDescriptor.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDescriptor.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDescriptor.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// create a desciptor for the pipeline state object.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStatedescriptor = {};
	pipelineStatedescriptor.InputLayout = { &inputDescriptor[0], (UINT)inputDescriptor.size() };
	pipelineStatedescriptor.pRootSignature = mRootSignature.get();
	pipelineStatedescriptor.VS = { reinterpret_cast<UINT8*>(mVertexShader->GetBufferPointer()), mVertexShader->GetBufferSize() };
	pipelineStatedescriptor.PS = { reinterpret_cast<UINT8*>(mPixelShader->GetBufferPointer()), mPixelShader->GetBufferSize() };
	pipelineStatedescriptor.RasterizerState = rasterizerDescriptor;
	pipelineStatedescriptor.BlendState = blendDescriptor;
	pipelineStatedescriptor.DepthStencilState.DepthEnable = false;
	pipelineStatedescriptor.DepthStencilState.StencilEnable = false;
	pipelineStatedescriptor.SampleMask = UINT_MAX;
	pipelineStatedescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStatedescriptor.NumRenderTargets = 1;
	pipelineStatedescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStatedescriptor.SampleDesc.Count = 1;

	// create a new graphics pipeline state with the above descriptors.
	check_hresult(mDevice->CreateGraphicsPipelineState(&pipelineStatedescriptor, __uuidof(ID3D12PipelineState), mPipelineState.put_void()));
	mPipelineState->SetName(L"mPipelineState");

	// create a command list for the rendering.
	check_hresult(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[0].get(), mPipelineState.get(), __uuidof(ID3D12GraphicsCommandList), mCommandList.put_void()));
	mCommandList->SetName(L"mCommandList");

	// create synchronization objects.
	check_hresult(mDevice->CreateFence((UINT64) 0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), mFence.put_void()));
	mFenceValues[0]++;
	mFenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (mFenceEvent == nullptr) {
		throw_last_error();
	}
}

void RenderingCtx::SetWindow(CoreWindow const&)
{
	// ...
}

void RenderingCtx::D3DInit()
{
	// enable the D3D12 debug layer in debug builds.
	#if defined(_DEBUG)
	com_ptr<ID3D12Debug> debugController;
	check_hresult(D3D12GetDebugInterface(UUID_PPV(debugController)));
	debugController->EnableDebugLayer();
	#endif

	// create a DXGI factory to enumerate system devices.
	UINT dxgiFactoryFlags = (UINT)0;
	#if defined(_DEBUG)
	dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
	#endif
	check_hresult(CreateDXGIFactory2(dxgiFactoryFlags, UUID_PPV(mDXGIFactory)));

	// create a D3D12 device.
	D3DCreateDevice();

	// TODO ....
}

void RenderingCtx::D3DCreateDevice()
{
	// find the adapter which supports Direct3D 12 and has the most memory.
	SIZE_T maxMemory = (SIZE_T)0;
	com_ptr<IDXGIAdapter1> tempAdapter;
	com_ptr<IDXGIAdapter1> dxgiAdapter;
	for (auto i = 0u; mDXGIFactory->EnumAdapters1(i, tempAdapter.put()) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC1 desc;
		tempAdapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		if (FAILED(D3D12CreateDevice(tempAdapter.get(), D3D_FEATURE_LEVEL_12_0, UUID_PPV(tempAdapter))))
			continue;
		if (desc.DedicatedVideoMemory > maxMemory) {
			maxMemory = desc.DedicatedVideoMemory;
			dxgiAdapter = tempAdapter;
		}
	}
	mDXGIAdapter = dxgiAdapter;

	// create D3D12 device for the target adapter (fallback to WARP in debug builds especially in Xbox One).
	auto hresult = D3D12CreateDevice(mDXGIAdapter.get(), D3D_FEATURE_LEVEL_12_0, UUID_PPV(mDXGIAdapter));
	#if defined(_DEBUG)
	if (FAILED(hresult)) {
		check_hresult(mDXGIFactory->EnumWarpAdapter(UUID_PPV(mDXGIAdapter)));
		hresult = D3D12CreateDevice(mDXGIAdapter.get(), D3D_FEATURE_LEVEL_12_0, UUID_PPV(mDXGIAdapter));
	}
	#endif
	check_hresult(hresult);
}
