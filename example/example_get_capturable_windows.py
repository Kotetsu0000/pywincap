import cv2
from pywincap import get_hwnd_by_title

def test_get_hwnd_by_title():
    # 1. キャプチャ可能なウィンドウの一覧を取得
    target_title = "タスク マネージャー" # 要: タスクマネージャーの起動
    target_hwnd = get_hwnd_by_title(target_title)

    # 2. HWNDが取得できたか確認
    assert target_hwnd is not None, f"HWND for '{target_title}' not found."

    print(f"HWND for '{target_title}': {target_hwnd}")

if __name__ == "__main__":
    test_get_hwnd_by_title()
    print("Test completed successfully.")