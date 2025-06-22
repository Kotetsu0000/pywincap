import sys
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

if sys.platform != "win32":
    raise RuntimeError("This package is only supported on Windows.")

# Windows 10 バージョン1903 (19H1) 以降を明示的に指定
WIN_VERSION = 0x0A00000D  # 1903 (19H1)

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
        include_dirs=[
            #r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0",
            #r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\shared",
            #r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt",
            r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\cppwinrt",
        ],
        define_macros=[
            ("WINVER", WIN_VERSION),
            ("_WIN32_WINNT", WIN_VERSION)
        ],
    ),
]

setup(
    name="pywincap",
    version="0.1.0",
    author="Your Name",
    author_email="your.email@example.com",
    description="A Python library to capture window contents on Windows using Windows.Graphics.Capture.",
    long_description=open('README.md').read(),
    long_description_content_type='text/markdown',
    package_dir={"": "src"},
    packages=["pywincap"],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
    install_requires=[
        "numpy",
    ],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
    ],
)
