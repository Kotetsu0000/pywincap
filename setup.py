import sys
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

if sys.platform != "win32":
    raise RuntimeError("This package is only supported on Windows.")

ext_modules = [
    Pybind11Extension(
        "pywincap._core",
        [
            "src/pywincap/main.cpp",
            "src/pywincap/WindowCapture.cpp",
        ],        extra_compile_args=["/bigobj", "/std:c++17"],  # C++17への変更
        extra_link_args=[
            "d3d11.lib",
            "dxgi.lib",
            "dwmapi.lib",
            "windowsapp.lib",
            "user32.lib",  # EnumWindows, GetWindowTextW などのための追加
        ],
    ),
]

setup(
    name="pywincap",
    version="1.0.0",
    author="Kotetsu0000",
    description="A Python library to capture window contents on Windows using Windows.Graphics.Capture.",
    package_dir={"": "src"},
    packages=["pywincap"],
    package_data={"pywincap": ["__init__.pyi", "_core.pyi"]},
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
    install_requires=[
        "numpy",
        "opencv-python"
    ],
)
