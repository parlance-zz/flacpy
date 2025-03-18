from setuptools import setup, Extension
import os
import platform
import numpy as np  # Import NumPy to get include directory

extra_compile_args = []
extra_link_args = []

# Platform-specific optimizations
if platform.system() == "Linux":
    extra_compile_args.extend(["-O3", "-march=native", "-ftree-vectorize"])
elif platform.system() == "Darwin":
    extra_compile_args.extend(["-O3", "-march=native"])
elif platform.system() == "Windows":
    extra_compile_args.extend(["/O2", "/arch:AVX2"])

flacpy_module = Extension(
    'flacpy',
    sources=['src/flacpy.cpp'],
    include_dirs=[
        '/usr/local/include',
        np.get_include(),  # Add NumPy's include directory
    ],
    library_dirs=['/usr/local/lib'],
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
    python_requires='>=3.6',
    install_requires=['numpy'],  # Add NumPy as a dependency
)