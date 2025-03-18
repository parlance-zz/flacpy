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


# filepath: /home/parlance/flacpy/examples/basic_usage.py
import numpy as np
import flacpy  # Import the FLAC Python extension

def main():
    # Example usage of the FLAC Python extension
    print("FLAC Python Extension Example")

    # Assuming flacpy has a function to read a FLAC file
    # Replace 'example.flac' with your actual FLAC file path
    flac_file_path = 'example.flac'
    
    try:
        # Load the FLAC file
        audio_data = flacpy.load_flac(flac_file_path)
        print("Loaded audio data:", audio_data)

        # Process the audio data (this is just a placeholder)
        processed_data = flacpy.process_audio(audio_data)
        print("Processed audio data:", processed_data)

    except Exception as e:
        print("An error occurred:", e)

if __name__ == "__main__":
    main()