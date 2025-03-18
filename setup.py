from setuptools import setup, Extension, find_packages
from pathlib import Path
import os
import platform
import numpy as np  # numpy import to get include directory

extra_compile_args = []
extra_link_args = []

flac_include_dir = os.environ.get("FLAC_INCLUDE_DIR", "/usr/include")
flac_lib_dir = os.environ.get("FLAC_LIB_DIR", "/usr/lib/x86_64-linux-gnu")

# platform-specific compiler optimization flags
if platform.system() == "Linux":
    extra_compile_args.extend(["-O3", "-march=native"])
elif platform.system() == "Darwin":
    extra_compile_args.extend(["-O3", "-march=native"])
elif platform.system() == "Windows":
    extra_compile_args.extend(["/O2", "/GL", "/LTCG", "/arch:AVX2"])

# debug info
print(f"Using FLAC_INCLUDE_DIR: {flac_include_dir}")
print(f"Using FLAC_LIB_DIR: {flac_lib_dir}")

flacpy_module = Extension(
    "flacpy._flacpy", 
    sources=["src/flacpy.cpp"],
    include_dirs=[
        flac_include_dir,
        np.get_include(),
    ],
    library_dirs=[flac_lib_dir],
    libraries=["FLAC", "FLAC++"],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    language="c++",
)

setup(
    name="flacpy",
    version="0.1.0",
    description="High-performance Python extension for FLAC file i/o",
    long_description= (Path(__file__).parent / "README.md").read_text(),
    long_description_content_type="text/markdown",
    url="https://github.com/parlance-zz/flacpy",
    license_files=["LICENSE"],
    author="parlance-zz",
    author_email="parlance@fifth-harmonic.com",
    ext_modules=[flacpy_module],
    packages=find_packages(),
    package_data={
        "flacpy": ["py.typed", "*.pyi"],
    },
    include_package_data=True,
    python_requires=">=3.11",
    install_requires=["numpy"],  # Add NumPy as a dependency
    zip_safe=False,  # This is important for proper type detection
)