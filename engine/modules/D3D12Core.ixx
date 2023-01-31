module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <wrl.h>
export module Render.D3D12Core;

import Render.Concepts;
import Render.Error;

using namespace render::error;
using namespace render::concepts;
using namespace Microsoft::WRL;

// global variable
ID3D12Device9 *main_device { nullptr };
IDXGIFactory7 *dxgi_factory{ nullptr };

void Release(Releaseable auto* p_obj);
IDXGIAdapter4* DetectHighestPerformanceAdapter();
void DestructGlobalResource();

namespace render::core {
    class D3D12Core {
    public:
        void Initialize()
        {
            Failed(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgi_factory)));
            ComPtr<IDXGIAdapter4> adapter;
            adapter.Attach(DetectHighestPerformanceAdapter());
            Failed(adapter);
            // check feature level
            // create device
            Failed(D3D12CreateDevice(adapter.Get(),
                                     D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&main_device)));
            main_device->SetName(L"Demo Device");
        }
    };
}  // render::core

module : private;

void Release(Releaseable auto* p_obj) {
    if (p_obj != nullptr) {
        p_obj->Release();
    }
}

IDXGIAdapter4* DetectHighestPerformanceAdapter() {
    IDXGIAdapter4* result_adapter { nullptr };
    for (uint32_t index { 0 };
         dxgi_factory->EnumAdapterByGpuPreference(index,
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

void DestructGlobalResource() {
    Release(dxgi_factory);
    Release(main_device);
}