#pragma once

#include <cstdint>
#include <memory>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#define NOMINMAX
#include <Windows.h>

namespace py = pybind11;

class WindowCapture {
public:
    WindowCapture(HWND hwnd);
    ~WindowCapture();

    WindowCapture(const WindowCapture&) = delete;
    WindowCapture& operator=(const WindowCapture&) = delete;
    WindowCapture(WindowCapture&&) noexcept;
    WindowCapture& operator=(WindowCapture&&) noexcept;

    py::array_t<uint8_t> grab_frame();
    void close();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl;
};
