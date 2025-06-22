import cv2
from pywincap import get_capturable_windows, WindowCapture

def test_window_capture():
    # 1. キャプチャ可能なウィンドウの一覧を取得
    windows = get_capturable_windows()
    assert len(windows) > 0, "No capturable windows found."

    # 2. 最初のウィンドウをターゲットにする
    target_window = list(windows.items())[1]  # 例として2番目のウィンドウを選択
    target_hwnd, target_title = target_window
    print(f"Targeting window '{target_title}' with HWND {target_hwnd}")

    # 3. WindowCaptureオブジェクトを作成
    capture = WindowCapture(target_hwnd)

    try:
        # 4. フレームを取得
        frame = capture.grab_frame()
        assert frame is not None and frame.size > 0, "Failed to grab a frame."

        # 5. OpenCVで扱えるように変換 (BGRA -> BGR)
        bgr_frame = cv2.cvtColor(frame, cv2.COLOR_BGRA2BGR)

        # 6. ファイルに保存
        cv2.imwrite("screenshot.png", bgr_frame)
        print("Screenshot saved to screenshot.png")

    finally:
        # 7. リソースを解放
        capture.close()
        print("Capture session closed.")

if __name__ == "__main__":
    test_window_capture()
    print("Test completed successfully.")