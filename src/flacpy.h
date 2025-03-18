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


#ifndef FLACPY_H
#define FLACPY_H

#include <Python.h>
#include <FLAC/stream_decoder.h>
#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>
#include <vector>
#include <string>
#include <memory>

// type definition for FLACAudio object
typedef struct {
    PyObject_HEAD
    std::vector<int32_t>* buffer;
    unsigned int channels;
    unsigned int sample_rate;
    unsigned int bits_per_sample;
} FLACAudioObject;

extern PyTypeObject FLACAudioType;

// api function declarations
PyObject* flacpy_load(PyObject* self, PyObject* args, PyObject* kwargs);
PyObject* flacpy_save(PyObject* self, PyObject* args, PyObject* kwargs);

#endif // FLACPY_H