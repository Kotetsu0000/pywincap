#include "WindowCapture.h"
#include <iostream>
#include <mutex>

// 1. initguid.h: COMインターフェースのGUIDを定義するために、
//    cppファイルの中で一度だけ、他のヘッダーの前にインクルードする。
#include <initguid.h>

// 2. 従来のWindows SDKヘッダー
#include <d3d11.h>
#include <dxgi1_2.h>

// 3. C++/WinRTのベースヘッダーと、具体的な射影ヘッダー
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <Unknwn.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace WinCapInterop {
    #ifndef __IDirect3DDxgiInterfaceAccess_INTERFACE_DEFINED__
    #define __IDirect3DDxgiInterfaceAccess_INTERFACE_DEFINED__
    MIDL_INTERFACE("a9b3d012-3df2-4ee3-b8d1-8695f457d3c1")
    IDirect3DDxgiInterfaceAccess : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInterface(
            REFIID iid,
            void **p
            ) = 0;
    };
    #endif // __IDirect3DDxgiInterfaceAccess_INTERFACE_DEFINED__
}

class WindowCapture::Impl {
public:
    Impl(HWND hwnd);
    ~Impl();

    void close();
    py::array_t<uint8_t> grab_frame();

private:
    void on_frame_arrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args);

    HWND _hwnd;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice _device{ nullptr };
    winrt::com_ptr<ID3D11Device> _d3d_device;
    winrt::com_ptr<ID3D11DeviceContext> _d3d_context;

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem _item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool _frame_pool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession _session{ nullptr };
    
    winrt::com_ptr<ID3D11Texture2D> _last_frame;
    std::mutex _frame_mutex;
    bool _is_closed = false;
};

WindowCapture::Impl::Impl(HWND hwnd) : _hwnd(hwnd) {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    winrt::check_hresult(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        creation_flags, feature_levels, ARRAYSIZE(feature_levels),
        D3D11_SDK_VERSION, _d3d_device.put(), nullptr, _d3d_context.put()));

    winrt::com_ptr<IDXGIDevice> dxgi_device = _d3d_device.as<IDXGIDevice>();
    winrt::com_ptr<IInspectable> d3d_inspectable;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device.get(), d3d_inspectable.put()));
    _device = d3d_inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
    winrt::check_hresult(interop_factory->CreateForWindow(_hwnd, winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), winrt::put_abi(_item)));

    if (!_item) throw std::runtime_error("Failed to create GraphicsCaptureItem.");

    _frame_pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(_device, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 1, _item.Size());
    _session = _frame_pool.CreateCaptureSession(_item);
    _frame_pool.FrameArrived({ this, &WindowCapture::Impl::on_frame_arrived });
    _session.StartCapture();
}

WindowCapture::Impl::~Impl() {
    close();
}

void WindowCapture::Impl::close() {
    if (_is_closed) return;
    if (_session) _session.Close();
    if (_frame_pool) _frame_pool.Close();
    _session = nullptr;
    _frame_pool = nullptr;
    _item = nullptr;
    _d3d_device = nullptr;
    _d3d_context = nullptr;
    _device = nullptr;
    _is_closed = true;
}

void WindowCapture::Impl::on_frame_arrived(
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
    winrt::Windows::Foundation::IInspectable const& /*args*/) {
    auto frame = sender.TryGetNextFrame();
    if (!frame) return;

    auto surface = frame.Surface();
    winrt::com_ptr<WinCapInterop::IDirect3DDxgiInterfaceAccess> access;
    winrt::check_hresult(surface.as<IInspectable>()->QueryInterface(
        __uuidof(WinCapInterop::IDirect3DDxgiInterfaceAccess), access.put_void()));

    winrt::com_ptr<ID3D11Texture2D> surface_texture;
    winrt::check_hresult(access->GetInterface(IID_PPV_ARGS(surface_texture.put())));

    std::lock_guard<std::mutex> lock(_frame_mutex);
    _last_frame = surface_texture;
}

py::array_t<uint8_t> WindowCapture::Impl::grab_frame() {
    if (_is_closed) throw std::runtime_error("Capture session is closed.");
    Sleep(50); 

    winrt::com_ptr<ID3D11Texture2D> frame_texture;
    {
        std::lock_guard<std::mutex> lock(_frame_mutex);
        if (!_last_frame) return py::array_t<uint8_t>();
        frame_texture = _last_frame;
    }

    D3D11_TEXTURE2D_DESC desc;
    frame_texture->GetDesc(&desc);
    D3D11_TEXTURE2D_DESC staging_desc = desc;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.BindFlags = 0;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_desc.MiscFlags = 0;

    winrt::com_ptr<ID3D11Texture2D> staging_texture;
    winrt::check_hresult(_d3d_device->CreateTexture2D(&staging_desc, nullptr, staging_texture.put()));
    _d3d_context->CopyResource(staging_texture.get(), frame_texture.get());

    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    winrt::check_hresult(_d3d_context->Map(staging_texture.get(), 0, D3D11_MAP_READ, 0, &mapped_resource));
    auto data_ptr = static_cast<uint8_t*>(mapped_resource.pData);

    py::array_t<uint8_t> img(py::array::ShapeContainer({(py::ssize_t)desc.Height, (py::ssize_t)desc.Width, 4}));
    auto buf = img.request();
    auto* py_ptr = static_cast<uint8_t*>(buf.ptr);

    for (uint32_t y = 0; y < desc.Height; ++y) {
        memcpy(py_ptr + y * desc.Width * 4, data_ptr + y * mapped_resource.RowPitch, desc.Width * 4);
    }

    _d3d_context->Unmap(staging_texture.get(), 0);
    return img;
}

WindowCapture::WindowCapture(HWND hwnd) : pimpl(std::make_unique<Impl>(hwnd)) {}
WindowCapture::~WindowCapture() = default;
WindowCapture::WindowCapture(WindowCapture&&) noexcept = default;
WindowCapture& WindowCapture::operator=(WindowCapture&&) noexcept = default;

py::array_t<uint8_t> WindowCapture::grab_frame() {
    if (!pimpl) throw std::runtime_error("Instance is moved or closed.");
    return pimpl->grab_frame();
}

void WindowCapture::close() {
    if (pimpl) {
        pimpl->close();
        pimpl.reset();
    }
}
