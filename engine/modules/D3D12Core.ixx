module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl.h>
#include "../inc/d3dx12.h"
#include <vector>
export module Render.D3D12Core;

import Render.Error;
import Common;

using namespace render::error;
using namespace common;
using namespace Microsoft::WRL;

// global variable


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
  IDXGISwapChain3 *swap_chain { nullptr };
  ID3D12DescriptorHeap* rtv_heap{nullptr};
  ID3D12DescriptorHeap* dsv_heap{nullptr};

  std::vector<ID3D12Resource2*> rtv_resources;
  std::vector<ID3D12Resource2*> dsv_resources;
  UINT rtv_descriptor_size{0};
  UINT dsv_descriptor_size{0};

  ~D3D12PipelineObject() {
    Release(main_device, dxgi_factory, command_queue, swap_chain,
            rtv_heap, dsv_heap, rtv_resources, dsv_resources,
            command_allocator
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
        void InitialPipeline()
      {
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

        IDXGIAdapter4* DetectHighestPerformanceAdapter() const;
        void CreateBufferResource(ID3D12DescriptorHeap* heap_desc,std::vector<ID3D12Resource2*> buffer, UINT descriptor_size);

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