module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl.h>
export module Render.D3D12Core;

import Render.Concepts;
import Render.Error;
import Common;

using namespace render::error;
using namespace render::concepts;
using namespace Microsoft::WRL;

// global variable


void Release(Releaseable auto* p_obj);
void Release(Releaseable auto* ... p_objs);


struct D3D12PipelineObject {
  ID3D12Device9 *main_device { nullptr };
  IDXGIFactory7 *dxgi_factory{ nullptr };
  ID3D12CommandQueue *command_queue{ nullptr };
  IDXGISwapChain1 *swap_chain { nullptr };
  ~D3D12PipelineObject() {
    Release(main_device, dxgi_factory, command_queue);
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
          Failed(adapter);
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

          Failed(m_pipeline.dxgi_factory->CreateSwapChainForHwnd(m_pipeline.command_queue, m_window_hwnd, &swap_chain_desc, nullptr, nullptr, &m_pipeline.swap_chain));
        }

        }

        IDXGIAdapter4* DetectHighestPerformanceAdapter();

      private:
        //----pipeline-----object-----
        D3D12PipelineObject m_pipeline;
        HWND m_window_hwnd;
        RenderCoreCommonData m_data;
    };

}  // render::core

module : private;

void Release(Releaseable auto* p_obj) {
    if (p_obj != nullptr) {
        p_obj->Release();
    }
}
void Release(Releaseable auto* ... p_objs) {
    (Release(p_objs), ...);
}
IDXGIAdapter4* render::core::D3D12Core::DetectHighestPerformanceAdapter() {
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
