module;
#include <cstdint>
#include <wrl.h>
#include "../inc/d3dx12.h"
#include <vector>
#include <filesystem>
#include <d3dcompiler.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
export module Render.D3D12Core;

import Render.Error;
import Common;

using namespace render::error;
using namespace common;
using namespace Microsoft::WRL;
namespace fs = std::filesystem;

// Temporary
struct Vertex {
  XMFLOAT3 position;
  XMFLOAT4 color;
};


void Release(Releaseable auto p_obj);
void Release(ReleaseableContainer auto container);
void Release(OneOrContainerReleasable auto ... p_objs);


// todo: abstract
// todo: why not use smart pointer? Because I am too lazy to change.
struct D3D12PipelineObject {
  ID3D12Device9 *main_device { nullptr };
  IDXGIFactory7 *dxgi_factory{ nullptr };
  ID3D12CommandQueue *command_queue{ nullptr };
  ID3D12CommandAllocator *command_allocator {nullptr};
  ID3D12GraphicsCommandList1 *command_list {nullptr};
  IDXGISwapChain3 *swap_chain { nullptr };
  ID3D12DescriptorHeap* rtv_heap{nullptr};
  ID3D12DescriptorHeap* dsv_heap{nullptr};
  ID3D12RootSignature* root_signature{nullptr};
  ID3D12PipelineState *pipeline_state {nullptr};
  ID3D12Fence1 *fence{nullptr};
  uint8_t fence_value = 0;
  HANDLE fence_event;


  ID3D12Resource2 *vertex_buffer{nullptr};
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;

  std::vector<ID3D12Resource2*> rtv_resources;
  std::vector<ID3D12Resource2*> dsv_resources;
  UINT rtv_descriptor_size{0};
  UINT dsv_descriptor_size{0};

  ~D3D12PipelineObject() {
    Release(main_device, dxgi_factory, command_queue, swap_chain,
            rtv_heap, dsv_heap, rtv_resources, dsv_resources,
            command_allocator, root_signature, pipeline_state
            );
  }
};
// todo: add debug layers

namespace render::core {
    class D3D12Core {
    public:
      void OnInit() {
        InitialPipeline();
      }
    private:
      void InitialPipeline() {
        // create device
        {
          Failed(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_pipeline.dxgi_factory)));
          ComPtr<IDXGIAdapter4> adapter;
          adapter.Attach(DetectHighestPerformanceAdapter());
          Failed(adapter ? S_OK : E_FAIL);
          // check feature level
          // create device
          Failed(D3D12CreateDevice(adapter.Get(),
                                   D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pipeline.main_device)));
          m_pipeline.main_device->SetName(L"Demo Device"); // for debug
        }
        // create command queue
        {
          D3D12_COMMAND_QUEUE_DESC command_queue_desc{};
          command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
          command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
          command_queue_desc.NodeMask = 0;
          command_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

          Failed(m_pipeline.main_device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&m_pipeline.command_queue)));
        }
        // create swap chain
        {
          DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
          swap_chain_desc.Width = m_data.resolution_width;
          swap_chain_desc.Height = m_data.resolution_height;
          swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
          swap_chain_desc.SampleDesc.Count = 1;
          swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
          swap_chain_desc.BufferCount = m_data.buffer_count;
          swap_chain_desc.Scaling = DXGI_SCALING_STRETCH;
          swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;  // ? flip somethings ?
          swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
          Failed(m_pipeline.dxgi_factory->CreateSwapChainForHwnd(m_pipeline.command_queue, m_window_hwnd, &swap_chain_desc, nullptr, nullptr,
              reinterpret_cast<IDXGISwapChain1 **>(&m_pipeline.swap_chain)));
          m_data.current_buffer_index = m_pipeline.swap_chain->GetCurrentBackBufferIndex();
        }
        // creat descriptor heap for rtv and dsv heap
        // todo: Extracting the creation descriptor heap into a function
        {
          D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{};
          descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
          descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
          descriptor_heap_desc.NumDescriptors = m_data.buffer_count;
          descriptor_heap_desc.NodeMask = 0;
          Failed(m_pipeline.main_device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&m_pipeline.rtv_heap)));
          descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
          Failed(m_pipeline.main_device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&m_pipeline.dsv_heap)));

          m_pipeline.rtv_descriptor_size = m_pipeline.main_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
          m_pipeline.dsv_descriptor_size = m_pipeline.main_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        }
        // create buffer resource
        {
          CreateBufferResource(m_pipeline.rtv_heap, m_pipeline.rtv_resources, m_pipeline.rtv_descriptor_size);
          CreateBufferResource(m_pipeline.dsv_heap, m_pipeline.dsv_resources, m_pipeline.dsv_descriptor_size);
        }
        // create command allocator
        {
          Failed(m_pipeline.main_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pipeline.command_allocator)));
        }
      }
      void LoadAssets() {
        // Create root signature
        {
          CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
          root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
          ID3DBlob *serialized_root_signature {nullptr};
          Failed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &serialized_root_signature, nullptr));
          Failed(m_pipeline.main_device->CreateRootSignature(0,
                                                             serialized_root_signature->GetBufferPointer(),
                                                             serialized_root_signature->GetBufferSize(),
                                                             IID_PPV_ARGS(&m_pipeline.root_signature)));
          Release(serialized_root_signature);  // why not use smart pointer?!
        }
        // create pipeline states object
        {
          ID3DBlob* vertex_shader = CompileShader("Shader.hlsl", "vs_5_0", "VS_main");
          ID3DBlob* pixel_shader  = CompileShader("Shader.hlsl", "ps_5_0", "PS_main");
          D3D12_INPUT_ELEMENT_DESC  input_element_desc[] =
              {
                  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                  { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
              };
          D3D12_GRAPHICS_PIPELINE_STATE_DESC graphics_pipeline_state{};
          graphics_pipeline_state.pRootSignature = m_pipeline.root_signature;
          graphics_pipeline_state.VS = CD3DX12_SHADER_BYTECODE(vertex_shader);
          graphics_pipeline_state.PS = CD3DX12_SHADER_BYTECODE(pixel_shader);
          graphics_pipeline_state.SampleMask = UINT_MAX;
          graphics_pipeline_state.DepthStencilState = {
              .DepthEnable = true,
              .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
              .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
              .StencilEnable = false
          };
          graphics_pipeline_state.InputLayout = {
              .pInputElementDescs = input_element_desc,
              .NumElements = _countof(input_element_desc)
          };
          graphics_pipeline_state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
          graphics_pipeline_state.NumRenderTargets = 1;
          graphics_pipeline_state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
          graphics_pipeline_state.SampleDesc.Count = 1;
          graphics_pipeline_state.DSVFormat = DXGI_FORMAT_D16_UNORM;
          graphics_pipeline_state.NodeMask = 0;


          //  Temporary Zeroing or default;
//          graphics_pipeline_state.StreamOutput {};
          graphics_pipeline_state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
          graphics_pipeline_state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

          Failed(m_pipeline.main_device->CreateGraphicsPipelineState(&graphics_pipeline_state, IID_PPV_ARGS(&m_pipeline.pipeline_state)));
          Release(vertex_shader, pixel_shader);
        }
        // Create command list
        {
          // createCommandList1
          Failed(m_pipeline.main_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                            m_pipeline.command_allocator, m_pipeline.pipeline_state, IID_PPV_ARGS(&m_pipeline.command_list)));
          Failed(m_pipeline.command_list->Close());
        }
        // vertex buffer
        // todo abstract
        {
          Vertex vertex_buffer = {
              { { 0.0f, 0.25f   , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
              { { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
              { { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
          };

          const uint8_t  vertex_buffer_size = sizeof(vertex_buffer);
          Failed(m_pipeline.main_device->CreateCommittedResource(
              &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
              D3D12_HEAP_FLAG_NONE,
              &CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
              D3D12_RESOURCE_STATE_GENERIC_READ,
              nullptr,
              IID_PPV_ARGS(&m_pipeline.vertex_buffer)));

          // Copy the triangle data to the vertex buffer.
          UINT8* p_vertex_data;
          CD3DX12_RANGE read_range(0, 0);        // We do not intend to read from this resource on the CPU.
          Failed(m_pipeline.vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&p_vertex_data)));
          memcpy(p_vertex_data, vertex_buffer, vertex_buffer_size);
          m_pipeline.vertex_buffer->Unmap(0, nullptr);

          // Initialize the vertex buffer view.
          m_vertexBufferView.BufferLocation = m_pipeline.vertex_buffer->GetGPUVirtualAddress();
          m_vertexBufferView.StrideInBytes = sizeof(Vertex);
          m_vertexBufferView.SizeInBytes = vertex_buffer_size;
        }

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        {
          Failed(m_pipeline.main_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pipeline.fence)));
          m_pipeline.fence_value = 1;

          // Create an event handle to use for frame synchronization.
          m_pipeline.fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
          if (m_fenceEvent == nullptr)
          {
            Failed(HRESULT_FROM_WIN32(GetLastError()));
          }
        }

      }

        [[nodiscard]] IDXGIAdapter4* DetectHighestPerformanceAdapter() const;
        void CreateBufferResource(ID3D12DescriptorHeap* heap_desc,std::vector<ID3D12Resource2*> buffer, UINT descriptor_size);
        [[nodiscard]] ID3DBlob* CompileShader(const fs::path& shader_path, const std::string& compiler_target, const std::string& shader_entry_point);

      private:
        //----pipeline-----object-----
        D3D12PipelineObject m_pipeline;
        HWND m_window_hwnd;
        RenderCoreCommonData m_data;
    };

}  // render::core

module : private;

void Release(Releaseable auto p_obj) {
    if (p_obj != nullptr) {
        p_obj->Release();
        p_obj = nullptr;
    }
}
void Release(ReleaseableContainer auto container) {
    for(auto element : container) {
        Release(element);
    }
}
void Release(OneOrContainerReleasable auto ... p_objs) {
    (Release(p_objs), ...);
}
IDXGIAdapter4* render::core::D3D12Core::DetectHighestPerformanceAdapter() const {
    IDXGIAdapter4* result_adapter { nullptr };
    for (uint32_t index { 0 };
         m_pipeline.dxgi_factory->EnumAdapterByGpuPreference(index,
                                                                   DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                                   IID_PPV_ARGS(&result_adapter));
         ++index) {
        if (SUCCEEDED(D3D12CreateDevice(result_adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device9), nullptr))) {
            return result_adapter;
        }
        Release(result_adapter);
    }
    return nullptr;
}

void render::core::D3D12Core::CreateBufferResource(
    ID3D12DescriptorHeap* heap_desc,std::vector<ID3D12Resource2*> buffer,
    UINT descriptor_size) {
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap_desc->GetCPUDescriptorHandleForHeapStart());

    for (uint8_t index = 0; index < m_data.buffer_count; ++index) {
        Failed(m_pipeline.swap_chain->GetBuffer(index, IID_PPV_ARGS(&buffer[index])));
        m_pipeline.main_device->CreateRenderTargetView(buffer[index], nullptr, handle);
        handle.Offset(1, descriptor_size);
    }

}
ID3DBlob *render::core::D3D12Core::CompileShader(
    const fs::path &shader_path,
    const std::string &compile_target, const std::string &shader_entry_point) {
    ID3DBlob *result{nullptr};
    Failed(D3DCompileFromFile(shader_path.c_str(), nullptr, nullptr, shader_entry_point.c_str(), compile_target.c_str(), D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &result, nullptr));
    return result;
}
