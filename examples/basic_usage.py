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

def main():
    # Example usage of the FLAC Python extension
    print("FLAC Python Extension Example")

    # Replace 'example.flac' with your actual FLAC file path
    flac_file_path = '/home/parlance/dualdiffusion/debug/test.flac'
    
    try:
        # Load the FLAC file
        result = flacpy.load(flac_file_path)
        print(f"Loaded audio data: {result['audio'].shape} samples at {result['sample_rate']}Hz")
        
        # Generate some test audio data if needed
        sample_rate = 44100
        duration = 2.0
        t = np.linspace(0, duration, int(sample_rate * duration), False)
        sine_wave = np.sin(2 * np.pi * 440 * t) * (2**15 - 1)
        audio_data = np.vstack((sine_wave, sine_wave)).T.astype(np.int32)
        
        # Save it back
        flacpy.save("output.flac", audio_data, sample_rate=44100)
        print("Saved audio file to output.flac")

    except Exception as e:
        print("An error occurred:", e)

if __name__ == "__main__":
    main()