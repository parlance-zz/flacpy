# FLACPy

FLACPy is a high-performance Python extension for handling FLAC (Free Lossless Audio Codec) file operations. This library is intended to resolve some of the shortcomings of existing popular libraries w.r.t. performance and metadata.

## Features

- Fastest possible FLAC file encoding and decoding with [libFLAC](https://github.com/xiph/flac). If loading using a start offset and length the library will only read and decode the required portion of the file.
- Arbitrary metadata can be read/written when loading/saving without redundant file i/o.
- Audio data uses [NumPy](https://github.com/numpy/numpy) arrays and metadata uses Python dicts.
- Save FLAC files with a specified bit depth and compression level
- Save FLAC files with seektables for fast loading when using a start offset and length.

## Installation

To install FLACPy, you can use pip:

```
pip install flacpy
```

Make sure you have the required dependencies installed, including NumPy and the FLAC libraries.

## Usage

Here is a simple example of how to use FLACPy:

```python
import flacpy

# Example usage of FLACPy functions
```

For more detailed usage, please refer to the `examples/basic_usage.py` file.

## Testing

To run the tests for FLACPy, navigate to the `tests` directory and execute:

```
pytest test_flacpy.py
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.

## Contributors
[parlance-zz](https://github.com/parlance-zz)

Feel free to reach out for any questions or contributions!