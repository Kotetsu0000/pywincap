# pywincap

pywincapは、Windows環境でウィンドウキャプチャを行うためのPythonライブラリです。Windows.Graphics.Captureを使用してウィンドウのコンテンツをキャプチャし、NumPy配列として返すことができます。

## 特徴

- 高速なウィンドウキャプチャ
- Windows 10/11での動作保証
- キャプチャ可能なウィンドウの列挙
- ウィンドウタイトルによるウィンドウハンドルの取得
- BGRAフォーマットのNumPy配列での画像取得

## 必要条件

- Windows 10/11
- Python 3.8以上
- Microsoft Visual C++ 14.0以上のビルドツール
- Windows 10 SDK 10.0.17763.0以上

## インストール方法

pipを使用してインストールできます：

```bash
pip install pywincap
```

または、ソースからビルドする場合：

```bash
git clone https://github.com/yourusername/pywincap.git
cd pywincap
pip install -e .
```

## 基本的な使い方

### キャプチャ可能なウィンドウの列挙

```python
from pywincap import get_capturable_windows

# キャプチャ可能なウィンドウの一覧を取得
windows = get_capturable_windows()
for hwnd, title in windows.items():
    print(f"HWND: {hwnd}, Title: {title}")
```

### ウィンドウタイトルからウィンドウハンドルを取得

```python
from pywincap import get_hwnd_by_title

# ウィンドウタイトルからウィンドウハンドルを取得
hwnd = get_hwnd_by_title("メモ帳")
print(f"メモ帳のウィンドウハンドル: {hwnd}")
```

### ウィンドウのキャプチャ

```python
import cv2
from pywincap import get_hwnd_by_title, WindowCapture

# ウィンドウハンドルを取得
hwnd = get_hwnd_by_title("メモ帳")

if hwnd:
    # WindowCaptureオブジェクトを作成
    capture = WindowCapture(hwnd)
    
    try:
        # フレームを取得
        frame = capture.grab_frame()
        
        # OpenCVで扱えるように変換 (BGRA -> BGR)
        bgr_frame = cv2.cvtColor(frame, cv2.COLOR_BGRA2BGR)
        
        # 画像を保存
        cv2.imwrite("screenshot.png", bgr_frame)
        print("スクリーンショットが保存されました: screenshot.png")
        
        # 画像を表示
        cv2.imshow("Screenshot", bgr_frame)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    
    finally:
        # リソースを解放
        capture.close()
else:
    print("指定したタイトルのウィンドウが見つかりませんでした。")
```

## API リファレンス

### `get_capturable_windows()`

キャプチャ可能なウィンドウの一覧を辞書形式で返します。

**戻り値**: `dict[int, str]` - キーがウィンドウハンドル（HWND）、値がウィンドウタイトルの辞書

### `get_hwnd_by_title(title: str)`

指定したタイトルを持つウィンドウのハンドルを返します。
完全一致するタイトルが見つからない場合は、部分一致するウィンドウを返します。

**引数**:
- `title` (str): 検索するウィンドウタイトル

**戻り値**: `int` - ウィンドウハンドル（HWND）。見つからない場合は0を返します。

### `class WindowCapture(hwnd: int)`

ウィンドウキャプチャを管理するクラス。

**コンストラクタ引数**:
- `hwnd` (int): キャプチャ対象のウィンドウハンドル

**メソッド**:
- `grab_frame()`: 現在のウィンドウフレームをキャプチャし、NumPy配列(BGRA形式)として返します。
- `close()`: キャプチャリソースを解放します。必ず使用終了後に呼び出してください。

## トラブルシューティング

1. **WindowCaptureエラー**
   - ウィンドウが最小化されていないか確認してください
   - 管理者権限が必要な場合があります

2. **画像が取得できない**
   - ウィンドウが実際に存在していることを確認してください
   - タイトルが完全に一致しているか確認してください

3. **ビルドエラー**
   - Visual C++ビルドツールがインストールされているか確認してください
   - Windows 10 SDKが適切にインストールされているか確認してください

## ライセンス

MITライセンスの下で公開されています。詳細については[LICENSE](LICENSE)ファイルを参照してください。