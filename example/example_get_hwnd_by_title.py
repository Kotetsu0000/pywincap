import cv2
from pywincap import get_capturable_windows

def test_get_capturable_windows():
    # 1. キャプチャ可能なウィンドウの一覧を取得
    windows = get_capturable_windows()
    
    # 2. 少なくとも1つのウィンドウがキャプチャ可能であることを確認
    assert len(windows) > 0, "No capturable windows found."

    # 3. 各ウィンドウのHWNDとタイトルを表示
    for hwnd, title in windows.items():
        print(f"HWND: {hwnd}, Title: {title}")
        assert isinstance(hwnd, int), f"HWND should be an integer, got {type(hwnd)}"
        assert isinstance(title, str), f"Title should be a string, got {type(title)}"
    
    print("All capturable windows passed the checks.")

if __name__ == "__main__":
    test_get_capturable_windows()
    print("Test completed successfully.")