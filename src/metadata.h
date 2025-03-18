/*
MIT License

Copyright (c) 2025 Christopher Friesen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef FLACPY_METADATA_H
#define FLACPY_METADATA_H

#include <Python.h>
#include <FLAC/metadata.h>

// convert FLAC metadata to Python dict
PyObject* metadata_to_dict(const FLAC__StreamMetadata* metadata);

// convert a sequence of FLAC metadata blocks to Python dict
PyObject* metadata_blocks_to_dict(const FLAC__StreamMetadata** metadata, unsigned int count);

// create FLAC metadata from Python dict
FLAC__StreamMetadata** dict_to_metadata_blocks(PyObject* dict, unsigned int* count);

#endif // FLACPY_METADATA_H