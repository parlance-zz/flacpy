# MIT License
#
# Copyright (c) 2025 Christopher Friesen
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import numpy as np
import flacpy
import os
import time

def get_test_data(sample_rate: int = 32000):
    duration = 2.0  # seconds
    frequency = 440.0  # Hz (A4)
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    sine_wave = np.sin(2 * np.pi * frequency * t) * (2**15 - 1)  # Scale to 16-bit
    audio_data = np.vstack((sine_wave, sine_wave)).T.astype(np.int32)  # Stereo
    print(f"Generated test audio: {audio_data.shape} samples, {sample_rate}Hz")
    return audio_data

def test_load_and_save():
    # Create test audio data - a simple sine wave
    sample_rate = 32000
    
    # Save the audio to a FLAC file
    input_file = "./tests/test.flac"
    output_file = "./tests/test_output.flac"
    metadata = {
        "vorbis_comment": {
            "TITLE": "Test Sine Wave",
            "ARTIST": "flacPy Test Script",
            "YEAR": "2025",
            "vendor": "flacPy"
        },
        "custom_key": "custom_val"
    }
    
    # Load the entire file
    print("Loading entire file...")
    start_time = time.time()
    result = flacpy.load(input_file)
    load_time = time.time() - start_time
    print(f"Full file load completed in {load_time:.3f} seconds")
    print(f"Loaded audio shape: {result['audio'].shape}")
    print(f"Sample rate: {result['sample_rate']}")
    
    sample_rate = result['sample_rate']
    audio_data = result["audio"]
    

    print(f"Saving audio to {output_file}...")
    start_time = time.time()
    flacpy.save(output_file, audio_data, metadata=metadata, 
                sample_rate=sample_rate, bits_per_sample=16)
    save_time = time.time() - start_time
    print(f"Save completed in {save_time:.3f} seconds")
    

    
    # Load just metadata
    print("Loading metadata only...")
    start_time = time.time()
    metadata_result = flacpy.load(input_file, metadata_only=True)
    metadata_time = time.time() - start_time
    print(f"Metadata load completed in {metadata_time:.3f} seconds")
    print(f"Metadata: {metadata_result['metadata']}")
    
    # Load a segment
    segment_start = int(sample_rate * 0.5)  # 0.5 seconds in
    segment_length = int(sample_rate * 0.25)  # 0.25 seconds long
    print(f"Loading segment from sample {segment_start} for {segment_length} samples...")
    start_time = time.time()
    segment = flacpy.load(input_file, start_sample=segment_start, num_samples=segment_length)
    segment_time = time.time() - start_time
    print(f"Segment load completed in {segment_time:.3f} seconds")
    print(f"Segment shape: {segment['audio'].shape}")
    
    # Clean up
    #os.remove(test_filename)
    print(f"Test file {input_file} removed")
    print("All tests completed!")

if __name__ == "__main__":
    test_load_and_save()