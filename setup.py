from setuptools import setup, Extension, find_packages
import os
import platform
import numpy as np  # Import NumPy to get include directory

extra_compile_args = []
extra_link_args = []

# Get include and lib paths from environment variables or use defaults
flac_include_dir = os.environ.get('FLAC_INCLUDE_DIR', '/usr/local/include')
flac_lib_dir = os.environ.get('FLAC_LIB_DIR', '/usr/local/lib')

# Platform-specific optimizations
if platform.system() == "Linux":
    extra_compile_args.extend(["-O3", "-march=native", "-ftree-vectorize"])
elif platform.system() == "Darwin":
    extra_compile_args.extend(["-O3", "-march=native"])
elif platform.system() == "Windows":
    extra_compile_args.extend(["/O2", "/arch:AVX2"])

flacpy_module = Extension(
    'flacpy._flacpy',  # Note the change here to make it a submodule
    sources=['src/flacpy.cpp'],
    include_dirs=[
        flac_include_dir,
        np.get_include(),  # Add NumPy's include directory
    ],
    library_dirs=[flac_lib_dir],
    libraries=['FLAC', 'FLAC++'],
    extra_compile_args=extra_compile_args,
    extra_link_args=extra_link_args,
    language='c++',
)

setup(
    name='flacpy',
    version='0.1.0',
    description='High-performance Python extension for FLAC file operations',
    author='parlance-zz',
    author_email='example@example.com',
    ext_modules=[flacpy_module],
    packages=find_packages(),
    package_data={
        'flacpy': ['py.typed', '*.pyi'],
    },
    include_package_data=True,
    python_requires='>=3.6',
    install_requires=['numpy'],  # Add NumPy as a dependency
    zip_safe=False,  # This is important for proper type detection
)