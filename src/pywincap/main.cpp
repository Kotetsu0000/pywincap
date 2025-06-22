#include <map>
#include <string>
#include <vector>
#include <Windows.h>
#include <codecvt>
#include <locale>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "WindowCapture.h"

namespace py = pybind11;

namespace pybind11 { namespace detail {
    template <> struct type_caster<HWND> {
    public:
        PYBIND11_TYPE_CASTER(HWND, const_name("int"));
        bool load(handle src, bool) {
            // Pythonのint型からHWNDに変換
            if (PyLong_Check(src.ptr())) {
                auto val = PyLong_AsLongLong(src.ptr());
                if (!PyErr_Occurred()) {
                    value = reinterpret_cast<HWND>(static_cast<intptr_t>(val));
                    return true;
                }
            }
            PyErr_Clear();
            return false;
        }
        static handle cast(HWND src, return_value_policy /* policy */, handle /* parent */) {
            return PyLong_FromLongLong(reinterpret_cast<intptr_t>(src));
        }
    };
}}

std::string to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str_to(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str_to[0], size_needed, NULL, NULL);
    return str_to;
}

std::wstring from_utf8(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr_to(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr_to[0], size_needed);
    return wstr_to;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    auto& windows = *reinterpret_cast<std::map<HWND, std::wstring>*>(lParam);
    const int length = GetWindowTextLength(hwnd);
    if (length == 0 || !IsWindowVisible(hwnd) || GetAncestor(hwnd, GA_ROOT) != hwnd) {
        return TRUE;
    }
    std::vector<wchar_t> buffer(length + 1);
    GetWindowTextW(hwnd, buffer.data(), length + 1);
    windows[hwnd] = buffer.data();
    return TRUE;
}

std::map<long long, std::string> get_capturable_windows() {
    std::map<HWND, std::wstring> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    
    std::map<long long, std::string> result;
    for(const auto& pair : windows) {
        result[reinterpret_cast<long long>(pair.first)] = to_utf8(pair.second);
    }
    return result;
}

long long get_hwnd_by_title(const std::string& title_utf8) {
    const std::wstring title_wide = from_utf8(title_utf8);
    HWND result = nullptr;
    
    std::map<HWND, std::wstring> windows;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
    
    for (const auto& pair : windows) {
        // 完全一致の場合
        if (pair.second == title_wide) {
            return reinterpret_cast<long long>(pair.first);
        }
        // 部分一致の場合
        if (pair.second.find(title_wide) != std::wstring::npos) {
            result = pair.first;
        }
    }
    
    return reinterpret_cast<long long>(result);
}

PYBIND11_MODULE(_core, m) {
    m.doc() = "Core implementation for pywincap";

    m.def("get_capturable_windows", &get_capturable_windows, 
        "Returns a dictionary of capturable windows {hwnd: title}");
      m.def("get_hwnd_by_title", &get_hwnd_by_title, py::arg("title"),
        "Returns the window handle as a long long integer for a window with the given title. "
        "Returns 0 if no window with the exact title is found.");

    py::class_<WindowCapture>(m, "WindowCapture")
        .def(py::init([](intptr_t hwnd) {
            return std::make_unique<WindowCapture>(reinterpret_cast<HWND>(hwnd));
        }), py::arg("hwnd"), "Initialize capture for a specific window handle.")
        .def("grab_frame", &WindowCapture::grab_frame, "Grabs a single frame and returns it as a NumPy array (BGRA).")
        .def("close", &WindowCapture::close, "Releases all capture resources.");
}
