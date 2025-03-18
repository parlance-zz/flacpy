from typing import Dict, Any, Union, Optional, TypedDict, List
import numpy as np
from numpy.typing import NDArray

class FLACAudio:
    """FLAC audio container class."""
    pass

class AudioData(TypedDict):
    audio: NDArray[np.int32]
    sample_rate: int
    bits_per_sample: int
    metadata: Dict[str, Any]

class MetadataData(TypedDict):
    metadata: Dict[str, Any]

def load(
    filename: str,
    start_sample: int = 0,
    num_samples: int = 0,
    metadata_only: bool = False
) -> Union[AudioData, MetadataData]:
    """
    Load a FLAC file with optional offset and length.
    
    Args:
        filename: Path to the FLAC file
        start_sample: Sample index to start loading from
        num_samples: Number of samples to load (0 = all remaining)
        metadata_only: If True, only load metadata without audio
        
    Returns:
        Dictionary containing audio data and/or metadata
    """
    ...

def save(
    filename: str,
    audio: NDArray[np.int32],
    metadata: Optional[Dict[str, Any]] = None,
    sample_rate: int = 44100,
    bits_per_sample: int = 16,
    compression_level: int = 5,
    metadata_pad_len: int = 0
) -> None:
    """
    Save audio data to a FLAC file with optional metadata.
    
    Args:
        filename: Path to save the FLAC file
        audio: Audio data as 2D NumPy array (frames × channels)
        metadata: Optional metadata dictionary
        sample_rate: Sample rate in Hz
        bits_per_sample: Bit depth of the saved audio
        compression_level: FLAC compression level (0-8)
        metadata_pad_len: Pad metadata blocks up to this length
    """
    ...