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


#define PY_ARRAY_UNIQUE_SYMBOL flacpy_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include "flacpy.h"
#include "metadata.h"
#include <numpy/arrayobject.h>
#include <FLAC++/decoder.h>
#include <FLAC++/encoder.h>
#include <FLAC++/metadata.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

// FLACAudio type definition
static PyMethodDef FLACAudio_methods[] = {
    {NULL, NULL, 0, NULL}
};

PyTypeObject FLACAudioType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "flacpy.FLACAudio",           /* tp_name */
    sizeof(FLACAudioObject),      /* tp_basicsize */
    0,                            /* tp_itemsize */
    0,                            /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "FLACAudio object",           /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    FLACAudio_methods,            /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    0,                            /* tp_new */
};

// FLAC decoder class for partial decoding
class PartialFLACDecoder : public FLAC::Decoder::File {
public:
    PartialFLACDecoder() : 
        buffer_(nullptr), 
        channels_(0), 
        bits_per_sample_(0), 
        sample_rate_(0),
        total_samples_(0),
        start_sample_(0),
        end_sample_(0),
        current_sample_(0),
        metadata_only_(false) {}

    void set_buffer(std::vector<int32_t>* buffer) { buffer_ = buffer; }
    
    void set_range(uint64_t start_sample, uint64_t length) {
        start_sample_ = start_sample;
        if (length == 0) {
            end_sample_ = UINT64_MAX;  // will be adjusted after metadata read
        } else {
            end_sample_ = start_sample + length;
        }
    }
    
    void set_metadata_only(bool metadata_only) { metadata_only_ = metadata_only; }
    
    unsigned get_channels() const { return channels_; }
    unsigned get_bits_per_sample() const { return bits_per_sample_; }
    unsigned get_sample_rate() const { return sample_rate_; }
    uint64_t get_total_samples() const { return total_samples_; }
    
    std::vector<FLAC__StreamMetadata*> metadata_blocks;

protected:
    // metadata callback required by FLAC::Decoder
    virtual ::FLAC__StreamDecoderWriteStatus write_callback(
            const ::FLAC__Frame* frame, const FLAC__int32* const buffer[]) override {
        
        if (metadata_only_) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
        }
        
        if (!buffer_) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        
        const uint64_t frame_first_sample = frame->header.number.sample_number;
        const unsigned frame_samples = frame->header.blocksize;
        const uint64_t frame_last_sample = frame_first_sample + frame_samples - 1;
        
        // skip frames entirely outside our target range
        if (frame_last_sample < start_sample_ || frame_first_sample >= end_sample_) {
            current_sample_ = frame_last_sample + 1;
            return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
        }
        
        // calculate overlap with our target range
        uint64_t start_offset = 0;
        uint64_t sample_count = frame_samples;
        
        if (frame_first_sample < start_sample_) {
            start_offset = start_sample_ - frame_first_sample;
            sample_count -= start_offset;
        }
        
        if (frame_last_sample >= end_sample_) {
            sample_count = end_sample_ - (frame_first_sample + start_offset);
        }
        
        // append data to our buffer
        for (unsigned s = 0; s < sample_count; s++) {
            for (unsigned c = 0; c < channels_; c++) {
                buffer_->push_back(buffer[c][start_offset + s]);
            }
        }
        
        current_sample_ = frame_first_sample + frame_samples;
        
        // if we've read all the samples we need, abort decoding
        if (current_sample_ >= end_sample_) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        
        return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
    }
    
    virtual void metadata_callback(const ::FLAC__StreamMetadata* metadata) override {
        if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
            channels_ = metadata->data.stream_info.channels;
            bits_per_sample_ = metadata->data.stream_info.bits_per_sample;
            sample_rate_ = metadata->data.stream_info.sample_rate;
            total_samples_ = metadata->data.stream_info.total_samples;
            
            // if end_sample wasn't specified, use total_samples
            if (end_sample_ == UINT64_MAX) {
                end_sample_ = total_samples_;
            }
            
            // allocate buffer with appropriate size (approximate)
            if (buffer_ && !metadata_only_) {
                uint64_t sample_count = end_sample_ - start_sample_;
                buffer_->reserve(sample_count * channels_);
            }
        }
        
        // make a copy of all metadata blocks
        FLAC__StreamMetadata* metadata_copy = FLAC__metadata_object_clone(metadata);
        if (metadata_copy) {
            metadata_blocks.push_back(metadata_copy);
        }
    }
    
    virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) override {
        std::cerr << "FLAC decoder error: " << FLAC__StreamDecoderErrorStatusString[status] << std::endl;
    }
    
private:
    std::vector<int32_t>* buffer_;
    unsigned channels_;
    unsigned bits_per_sample_;
    unsigned sample_rate_;
    uint64_t total_samples_;
    uint64_t start_sample_;
    uint64_t end_sample_;
    uint64_t current_sample_;
    bool metadata_only_;
};

// helper function to free metadata blocks
void free_metadata_blocks(std::vector<FLAC__StreamMetadata*>& blocks) {
    for (auto block : blocks) {
        if (block) {
            FLAC__metadata_object_delete(block);
        }
    }
    blocks.clear();
}

// convert metadata blocks to Python dict
PyObject* metadata_to_dict(const std::vector<FLAC__StreamMetadata*>& metadata_blocks) {
    PyObject* dict = PyDict_New();
    if (!dict) return NULL;
    
    for (const auto* metadata : metadata_blocks) {
        switch (metadata->type) {
            case FLAC__METADATA_TYPE_STREAMINFO:
            {
                const auto& info = metadata->data.stream_info;
                PyDict_SetItemString(dict, "min_blocksize", PyLong_FromLong(info.min_blocksize));
                PyDict_SetItemString(dict, "max_blocksize", PyLong_FromLong(info.max_blocksize));
                PyDict_SetItemString(dict, "min_framesize", PyLong_FromLong(info.min_framesize));
                PyDict_SetItemString(dict, "max_framesize", PyLong_FromLong(info.max_framesize));
                PyDict_SetItemString(dict, "sample_rate", PyLong_FromLong(info.sample_rate));
                PyDict_SetItemString(dict, "channels", PyLong_FromLong(info.channels));
                PyDict_SetItemString(dict, "bits_per_sample", PyLong_FromLong(info.bits_per_sample));
                PyDict_SetItemString(dict, "total_samples", PyLong_FromUnsignedLongLong(info.total_samples));
                break;
            }
            case FLAC__METADATA_TYPE_VORBIS_COMMENT:
            {
                const auto& vc = metadata->data.vorbis_comment;
                PyObject* comments = PyDict_New();
                
                // add vendor string
                if (vc.vendor_string.entry) {
                    PyDict_SetItemString(comments, "vendor", 
                        PyUnicode_FromStringAndSize((const char*)vc.vendor_string.entry, vc.vendor_string.length));
                }
                
                // add comments
                for (unsigned i = 0; i < vc.num_comments; i++) {
                    const FLAC__StreamMetadata_VorbisComment_Entry& entry = vc.comments[i];
                    if (entry.entry) {
                        std::string comment((const char*)entry.entry, entry.length);
                        size_t eq_pos = comment.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string field = comment.substr(0, eq_pos);
                            std::string value = comment.substr(eq_pos + 1);
                            PyDict_SetItemString(comments, field.c_str(), PyUnicode_FromString(value.c_str()));
                        }
                    }
                }
                
                PyDict_SetItemString(dict, "vorbis_comment", comments);
                break;
            }
            case FLAC__METADATA_TYPE_PICTURE:
            {
                const auto& pic = metadata->data.picture;
                PyObject* picture = PyDict_New();
                
                PyDict_SetItemString(picture, "type", PyLong_FromLong(pic.type));
                PyDict_SetItemString(picture, "mime_type", PyUnicode_FromString(pic.mime_type));
                PyDict_SetItemString(picture, "description", 
                    PyUnicode_FromStringAndSize((const char*)pic.description, strlen((const char*)pic.description)));
                PyDict_SetItemString(picture, "width", PyLong_FromLong(pic.width));
                PyDict_SetItemString(picture, "height", PyLong_FromLong(pic.height));
                PyDict_SetItemString(picture, "depth", PyLong_FromLong(pic.depth));
                PyDict_SetItemString(picture, "colors", PyLong_FromLong(pic.colors));
                
                PyObject* data = PyBytes_FromStringAndSize((const char*)pic.data, pic.data_length);
                PyDict_SetItemString(picture, "data", data);
                Py_DECREF(data);
                
                // add to pictures list
                PyObject* pictures;
                if (!PyDict_GetItemString(dict, "pictures")) {
                    pictures = PyList_New(0);
                    PyDict_SetItemString(dict, "pictures", pictures);
                    Py_DECREF(pictures);
                } else {
                    pictures = PyDict_GetItemString(dict, "pictures");
                }
                
                PyList_Append(pictures, picture);
                Py_DECREF(picture);
                break;
            }
            // tbd: handle other metadata types as needed...
        }
    }
    
    return dict;
}

// load a FLAC file with optional offset and length
PyObject* flacpy_load(PyObject* self, PyObject* args, PyObject* kwargs) {
    const char* filename;
    uint64_t start_sample = 0;
    uint64_t num_samples = 0;
    int metadata_only = 0;
    
    static const char* kwlist[] = {"filename", "start_sample", "num_samples", "metadata_only", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|KKp", const_cast<char**>(kwlist),
                                   &filename, &start_sample, &num_samples, &metadata_only)) {
        return NULL;
    }
    
    std::vector<int32_t> buffer;
    PartialFLACDecoder decoder;
    
    // set up decoder
    decoder.set_buffer(&buffer);
    decoder.set_range(start_sample, num_samples);
    decoder.set_metadata_only(metadata_only != 0);
    
    // initialize decoder
    FLAC__StreamDecoderInitStatus init_status = decoder.init(filename);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        PyErr_Format(PyExc_RuntimeError, "Failed to initialize FLAC decoder: %s",
                    FLAC__StreamDecoderInitStatusString[init_status]);
        return NULL;
    }
    
    // process metadata
    decoder.process_until_end_of_metadata();
    
    // process audio if needed
    if (!metadata_only) {
        // use seek table if possible for faster positioning
        if (start_sample > 0) {
            decoder.seek_absolute(start_sample);
        }
        
        // Decode audio data
        decoder.process_until_end_of_stream();
    }
    
    // create return value
    PyObject* result = PyDict_New();
    if (!result) {
        free_metadata_blocks(decoder.metadata_blocks);
        return NULL;
    }
    
    // add audio data if we requested it
    if (!metadata_only) {
        // get audio parameters
        unsigned channels = decoder.get_channels();
        unsigned bits_per_sample = decoder.get_bits_per_sample();
        unsigned sample_rate = decoder.get_sample_rate();
        
        npy_intp dims[2];
        dims[0] = buffer.size() / channels;  // number of frames
        dims[1] = channels;                  // number of channels
        
        PyObject* audio_array = PyArray_SimpleNew(2, dims, NPY_INT32);
        if (!audio_array) {
            Py_DECREF(result);
            free_metadata_blocks(decoder.metadata_blocks);
            return NULL;
        }
        
        // copy audio data to numpy array
        int32_t* data_ptr = static_cast<int32_t*>(PyArray_DATA(reinterpret_cast<PyArrayObject*>(audio_array)));
        std::copy(buffer.begin(), buffer.end(), data_ptr);
        
        // add audio data and parameters to result
        PyDict_SetItemString(result, "audio", audio_array);
        PyDict_SetItemString(result, "sample_rate", PyLong_FromLong(sample_rate));
        PyDict_SetItemString(result, "bits_per_sample", PyLong_FromLong(bits_per_sample));
        
        Py_DECREF(audio_array);
    }
    
    // add metadata to result
    PyObject* metadata = metadata_to_dict(decoder.metadata_blocks);
    PyDict_SetItemString(result, "metadata", metadata);
    Py_DECREF(metadata);
    
    // clean up metadata blocks
    free_metadata_blocks(decoder.metadata_blocks);
    
    return result;
}

// FLAC encoder class
class FLACEncoder : public FLAC::Encoder::File {
protected:
    // progress callback
    virtual void progress_callback(FLAC__uint64 bytes_written, FLAC__uint64 samples_written, 
                                 unsigned frames_written, unsigned total_frames_estimate) override {
        // could be used to report encoding progress
    }
};

// extract Vorbis comments from Python dict
FLAC__StreamMetadata* create_vorbis_comment_from_dict(PyObject* comments_dict) {
    if (!PyDict_Check(comments_dict)) {
        return nullptr;
    }
    
    FLAC__StreamMetadata* vorbis_comment = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    if (!vorbis_comment) {
        return nullptr;
    }
    
    PyObject* key, *value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(comments_dict, &pos, &key, &value)) {
        if (!PyUnicode_Check(key) || !PyUnicode_Check(value)) {
            continue;
        }
        
        const char* key_str = PyUnicode_AsUTF8(key);
        const char* value_str = PyUnicode_AsUTF8(value);
        
        if (strcmp(key_str, "vendor") == 0) {
            // set vendor string using the correct structure
            FLAC__StreamMetadata_VorbisComment_Entry entry;
            entry.entry = (FLAC__byte*)strdup(value_str);
            entry.length = strlen(value_str);
            
            FLAC__metadata_object_vorbiscomment_set_vendor_string(
                vorbis_comment,
                entry,
                true  // takes ownership of the memory
            );
        } else {
            // add regular comment
            std::string entry_text = std::string(key_str) + "=" + std::string(value_str);
            FLAC__StreamMetadata_VorbisComment_Entry comment;
            comment.entry = (FLAC__byte*)strdup(entry_text.c_str());
            comment.length = entry_text.length();
            
            FLAC__metadata_object_vorbiscomment_append_comment(
                vorbis_comment,
                comment,
                true  // takes ownership of the memory
            );
        }
    }
    
    return vorbis_comment;
}

// create picture metadata from Python dict
FLAC__StreamMetadata* create_picture_from_dict(PyObject* picture_dict) {
    if (!PyDict_Check(picture_dict)) {
        return nullptr;
    }
    
    FLAC__StreamMetadata* picture = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
    if (!picture) {
        return nullptr;
    }
    
    // extract picture type
    PyObject* type_obj = PyDict_GetItemString(picture_dict, "type");
    if (type_obj && PyLong_Check(type_obj)) {
        picture->data.picture.type = (FLAC__StreamMetadata_Picture_Type)PyLong_AsLong(type_obj);
    }
    
    // extract MIME type
    PyObject* mime_obj = PyDict_GetItemString(picture_dict, "mime_type");
    if (mime_obj && PyUnicode_Check(mime_obj)) {
        const char* mime_str = PyUnicode_AsUTF8(mime_obj);
        picture->data.picture.mime_type = strdup(mime_str);
    }
    
    // extract description
    PyObject* desc_obj = PyDict_GetItemString(picture_dict, "description");
    if (desc_obj && PyUnicode_Check(desc_obj)) {
        const char* desc_str = PyUnicode_AsUTF8(desc_obj);
        picture->data.picture.description = (FLAC__byte*)strdup(desc_str);
    }
    
    // extract dimensions
    PyObject* width_obj = PyDict_GetItemString(picture_dict, "width");
    if (width_obj && PyLong_Check(width_obj)) {
        picture->data.picture.width = PyLong_AsLong(width_obj);
    }
    
    PyObject* height_obj = PyDict_GetItemString(picture_dict, "height");
    if (height_obj && PyLong_Check(height_obj)) {
        picture->data.picture.height = PyLong_AsLong(height_obj);
    }
    
    PyObject* depth_obj = PyDict_GetItemString(picture_dict, "depth");
    if (depth_obj && PyLong_Check(depth_obj)) {
        picture->data.picture.depth = PyLong_AsLong(depth_obj);
    }
    
    PyObject* colors_obj = PyDict_GetItemString(picture_dict, "colors");
    if (colors_obj && PyLong_Check(colors_obj)) {
        picture->data.picture.colors = PyLong_AsLong(colors_obj);
    }
    
    // extract picture data
    PyObject* data_obj = PyDict_GetItemString(picture_dict, "data");
    if (data_obj && PyBytes_Check(data_obj)) {
        char* data_ptr;
        Py_ssize_t data_len;
        if (PyBytes_AsStringAndSize(data_obj, &data_ptr, &data_len) == 0) {
            picture->data.picture.data = (FLAC__byte*)malloc(data_len);
            if (picture->data.picture.data) {
                memcpy(picture->data.picture.data, data_ptr, data_len);
                picture->data.picture.data_length = data_len;
            }
        }
    }
    
    return picture;
}

// create metadata blocks from Python dict
std::vector<FLAC__StreamMetadata*> create_metadata_from_dict(PyObject* metadata_dict) {
    std::vector<FLAC__StreamMetadata*> blocks;
    
    if (!PyDict_Check(metadata_dict)) {
        return blocks;
    }
    
    // handle Vorbis comments
    PyObject* vorbis_comments = PyDict_GetItemString(metadata_dict, "vorbis_comment");
    if (vorbis_comments) {
        FLAC__StreamMetadata* vorbis_block = create_vorbis_comment_from_dict(vorbis_comments);
        if (vorbis_block) {
            blocks.push_back(vorbis_block);
        }
    }
    
    // handle pictures
    PyObject* pictures = PyDict_GetItemString(metadata_dict, "pictures");
    if (pictures && PyList_Check(pictures)) {
        Py_ssize_t num_pics = PyList_Size(pictures);
        for (Py_ssize_t i = 0; i < num_pics; i++) {
            PyObject* pic_dict = PyList_GetItem(pictures, i);
            FLAC__StreamMetadata* picture_block = create_picture_from_dict(pic_dict);
            if (picture_block) {
                blocks.push_back(picture_block);
            }
        }
    }
    
    // tbd: add other metadata types as needed...
    
    return blocks;
}

// save a FLAC file with optional metadata
PyObject* flacpy_save(PyObject* self, PyObject* args, PyObject* kwargs) {
    const char* filename;
    PyObject* audio_obj;
    PyObject* metadata_dict = NULL;
    int sample_rate = 44100;
    int bits_per_sample = 16;
    int compression_level = 5;  // default compression level (0-8)
    
    static const char* kwlist[] = {"filename", "audio", "metadata", "sample_rate", "bits_per_sample", "compression_level", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO|Oiii", const_cast<char**>(kwlist),
                                   &filename, &audio_obj, &metadata_dict,
                                   &sample_rate, &bits_per_sample, &compression_level)) {
        return NULL;
    }
    
    // ensure audio_obj is a NumPy array
    if (!PyArray_Check(audio_obj)) {
        audio_obj = PyArray_FROM_OTF(audio_obj, NPY_INT32, NPY_ARRAY_IN_ARRAY);
        if (!audio_obj) {
            PyErr_SetString(PyExc_TypeError, "Could not convert audio data to NumPy array");
            return NULL;
        }
    } else {
        Py_INCREF(audio_obj);
    }
    
    PyArrayObject* audio_array = reinterpret_cast<PyArrayObject*>(audio_obj);
    
    // validate array dimensions
    int ndim = PyArray_NDIM(audio_array);
    if (ndim != 2) {
        Py_DECREF(audio_obj);
        PyErr_SetString(PyExc_ValueError, "Audio data must be a 2D array (frames x channels)");
        return NULL;
    }
    
    // get array info
    npy_intp* dims = PyArray_DIMS(audio_array);
    unsigned num_samples = dims[0];     // number of frames
    unsigned num_channels = dims[1];    // number of channels
    
    // ensure we have int32 data
    if (PyArray_TYPE(audio_array) != NPY_INT32) {
        PyObject* tmp = PyArray_Cast(audio_array, NPY_INT32);
        Py_DECREF(audio_obj);
        audio_obj = tmp;
        audio_array = reinterpret_cast<PyArrayObject*>(audio_obj);
    }
    
    // get a pointer to the audio data
    int32_t* audio_data = static_cast<int32_t*>(PyArray_DATA(audio_array));
    
    // set up the encoder
    FLACEncoder encoder;
    encoder.set_verify(true);
    encoder.set_compression_level(compression_level);
    encoder.set_channels(num_channels);
    encoder.set_bits_per_sample(bits_per_sample);
    encoder.set_sample_rate(sample_rate);
    
    // add metadata if provided
    if (metadata_dict && PyDict_Check(metadata_dict)) {
        std::vector<FLAC__StreamMetadata*> metadata_blocks = create_metadata_from_dict(metadata_dict);
        
        if (!metadata_blocks.empty()) {
            encoder.set_metadata(metadata_blocks.data(), metadata_blocks.size());
        }
        
        // note: metadata_blocks ownership is transferred to the encoder, do not free here
    }
    
    // initialize the encoder
    FLAC__StreamEncoderInitStatus init_status = encoder.init(filename);
    if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        Py_DECREF(audio_obj);
        PyErr_Format(PyExc_RuntimeError, "Failed to initialize FLAC encoder: %s",
                    FLAC__StreamEncoderInitStatusString[init_status]);
        return NULL;
    }
    
    // prepare buffer for interleaved to non-interleaved conversion
    std::vector<FLAC__int32*> buffer(num_channels);
    std::vector<std::vector<FLAC__int32>> channel_data(num_channels);
    
    for (unsigned c = 0; c < num_channels; c++) {
        channel_data[c].resize(num_samples);
        buffer[c] = channel_data[c].data();
    }
    
    // de-interleave the audio data
    for (unsigned s = 0; s < num_samples; s++) {
        for (unsigned c = 0; c < num_channels; c++) {
            channel_data[c][s] = audio_data[s * num_channels + c];
        }
    }
    
    // encode the audio data
    bool ok = encoder.process(buffer.data(), num_samples);
    
    // clean up
    encoder.finish();
    Py_DECREF(audio_obj);

    if (!ok) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to encode audio data");
        return NULL;
    }   
    
    Py_RETURN_NONE;
}

// module method definitions
static PyMethodDef FLACPyMethods[] = {
    {"load", (PyCFunction)flacpy_load, METH_VARARGS | METH_KEYWORDS, 
     "Load a FLAC file with optional offset and length"},
    {"save", (PyCFunction)flacpy_save, METH_VARARGS | METH_KEYWORDS,
     "Save audio data to a FLAC file with optional metadata"},
    {NULL, NULL, 0, NULL}
};

// module definition
static struct PyModuleDef flacpymodule = {
    PyModuleDef_HEAD_INIT,
    "flacpy._flacpy",  // Module name
    "High performance FLAC file operations",  // Module doc string
    -1,  // Size of per-interpreter state or -1
    FLACPyMethods  // Method table
};

// module initialization function
PyMODINIT_FUNC PyInit__flacpy(void) {  // Note: double underscore before flacpy
    PyObject* m;
    
    // initialize NumPy
    import_array();
    
    // create the module
    m = PyModule_Create(&flacpymodule);
    if (m == NULL)
        return NULL;
    
    // initialize our custom types
    if (PyType_Ready(&FLACAudioType) < 0)
        return NULL;
    
    // add types to the module
    Py_INCREF(&FLACAudioType);
    PyModule_AddObject(m, "FLACAudio", (PyObject*)&FLACAudioType);
    
    return m;
}