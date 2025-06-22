"""
Core implementation for pywincap
"""
from __future__ import annotations
import numpy
__all__ = ['WindowCapture', 'get_capturable_windows', 'get_hwnd_by_title']
class WindowCapture:
    def __init__(self, hwnd: int) -> None:
        """
        Initialize capture for a specific window handle.
        """
    def close(self) -> None:
        """
        Releases all capture resources.
        """
    def grab_frame(self) -> numpy.ndarray[numpy.uint8]:
        """
        Grabs a single frame and returns it as a NumPy array (BGRA).
        """
def get_capturable_windows() -> dict[int, str]:
    """
    Returns a dictionary of capturable windows {hwnd: title}
    """
def get_hwnd_by_title(title: str) -> int:
    """
    Returns the window handle as a long long integer for a window with the given title. Returns 0 if no window with the exact title is found.
    """
