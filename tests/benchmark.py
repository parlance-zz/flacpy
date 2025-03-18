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
import time
import os

def generate_audio(duration, sample_rate=44100, channels=2):
    """Generate test audio data."""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    audio = []
    
    for i in range(channels):
        freq = 440 * (i + 1)  # Different frequency for each channel
        audio.append(np.sin(2 * np.pi * freq * t) * (2**15 - 1))
    
    return np.vstack(audio).T.astype(np.int32)

def run_benchmark():
    # Test parameters
    durations = [1, 5, 30]  # seconds
    sample_rate = 44100
    temp_file = "benchmark.flac"
    
    print("=== FLACPY PERFORMANCE BENCHMARK ===")
    
    for duration in durations:
        print(f"\n== Testing with {duration} seconds of audio ==")
        # Generate test data
        audio = generate_audio(duration, sample_rate)
        samples = audio.shape[0]
        channels = audio.shape[1]
        print(f"Generated {samples} samples, {channels} channels")
        
        # Test save performance
        print("\nSAVE PERFORMANCE:")
        start_time = time.time()
        flacpy.save(temp_file, audio, sample_rate=sample_rate, bits_per_sample=16)
        elapsed = time.time() - start_time
        file_size = os.path.getsize(temp_file) / 1024.0  # KB
        print(f"  Time: {elapsed:.3f} seconds")
        print(f"  Speed: {samples/elapsed:.1f} samples/sec")
        print(f"  File size: {file_size:.1f} KB")
        
        # Test full file load performance
        print("\nFULL FILE LOAD PERFORMANCE:")
        start_time = time.time()
        result = flacpy.load(temp_file)
        elapsed = time.time() - start_time
        print(f"  Time: {elapsed:.3f} seconds")
        print(f"  Speed: {samples/elapsed:.1f} samples/sec")
        
        # Test metadata-only load performance
        print("\nMETADATA LOAD PERFORMANCE:")
        start_time = time.time()
        _ = flacpy.load(temp_file, metadata_only=True)
        elapsed = time.time() - start_time
        print(f"  Time: {elapsed:.3f} seconds")
        
        # Test partial load performance (middle 10%)
        start_sample = samples // 2 - samples // 20
        length = samples // 10
        print(f"\nPARTIAL LOAD PERFORMANCE (10% from middle):")
        start_time = time.time()
        segment = flacpy.load(temp_file, start_sample=start_sample, num_samples=length)
        elapsed = time.time() - start_time
        print(f"  Time: {elapsed:.3f} seconds")
        print(f"  Speed: {length/elapsed:.1f} samples/sec")
        
    # Clean up
    os.remove(temp_file)
    print("\nBenchmark complete!")

if __name__ == "__main__":
    run_benchmark()