# DirectWrite CustomFont
DirectWriteの機能を使用して、

**フォントファイル（ttf形式など）からフォントを読み込んで描画を行うクラス**です。

DirectXなどで利用することができます。

## 機能
- **フォントファイルの読み込み**
- **カスタムフォントコレクションの作成**
- フォント名の取得
- フォントスタイル、サイズ、色の設定
- **文字列の描画**

## 紹介
**こちらのブログでも機能紹介、トラブルシューティング、メモを載せています。**
<https://islingtonsystem.hatenablog.jp/entry/DirectWrite_CustomFont_DirectX>

## メモ
開発環境ではDirectX11を使用してテストを行いました。

DirectX12でも利用可能とは思いますが、動作は未確認です。

また、D3DXMathや一部古い機能を使っています。

## 初期設定
1. `DirectWriteCustomFont.h`の`FontList::Fontpath`にフォントファイルのパスを書いてください。
2. `D3D11CreateDevice`もしくは`D3D11CreateDeviceAndSwapChain`関数の第4引数を以下に変更してください。

**D3D11_CREATE_DEVICE_BGRA_SUPPORT**

詳細は[こちら](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_create_device_flag)。
Direct2DとDirect3Dの共用に必要なので、Direct2D単体で使用する場合は必要ありません。


## 使い方
1. `DirectWriteCustomFont`クラスのインスタンスを作成します。
2. `Init`メソッドを使用して初期化処理を行います **（重要！）**
3. `FontData`クラスを宣言してフォント設定を保存します。
4. 必要に応じてフォントの設定を行います（例: フォント名、スタイル、サイズ）。
5. 変更を行った場合、`SetFont`メソッドでフォント設定を反映します。
6. `DrawString`メソッドを使用してテキストを描画します。


## 注意点
`FontData`クラスの`font`で指定するフォント名は、**フォントファイルの名前ではなくフォント名です。**
[`GetFontName`](#getfontnameint-num)で取得すると、ミスが無く指定できるのでおすすめです。


## サンプルコード
サンプルクラスは[こちら](/SampleCode/)に載っています。

```cpp
// DirectWriteCustomFontクラスの生成
Write = new DirectWriteCustomFont(&data);

// 初期化（SwapChainの取得は適宜お願いします）
Write->Init(Renderer::GetSwapChain());

// フォントデータを改変
data.fontSize = 60;
data.fontWeight = DWRITE_FONT_WEIGHT_ULTRA_BLACK;
data.Color = D2D1::ColorF(D2D1::ColorF::Red);
data.font = L"サンプル.ttf";

// フォントをセット
Write->SetFont(data);

// 描画
Write->DrawString("テスト", D3DXVECTOR2(90, 90), D2D1_DRAW_TEXT_OPTIONS_NONE);
```


## リファレンス
ヘッダーファイル内にもコメントで説明書きがあるので、困った際にはお読みください。

| メソッド  |  役割   |
| -------- | ----------  |
| [`Init`](#Init) | クラスの初期化を行います。|
| [`SetFont`](#SetFont) |  設定したフォントデータを反映します。|
| [`DrawString`](#DrawString) |  文字列の描画を行います。|
| [`GetFontName`](#GetFontName) |  指定した番号のフォント名を返します。|
| [`GetFontNameNum`](#GetFontNameNum) |  読み込んだフォントの最大数を返します。|
| [`GetFontFamilyName`](#GetFontFamilyName) |  フォント名を読み込み直します。|
| [`GetAllFontFamilyName`](#GetAllFontFamilyName) |  全てのフォント名を読み込み直します。|
| [`FontLoader`](#fontloader) |  指定されたパスのフォントを読み込み直します。|

---

## Init
- **全体の初期設定に使用します。**

呼び出しと同時に`FontList::FontPass`で指定されたフォントも読み込まれます。

##### パラメータ
- `IDXGISwapChain* swapChain`: Direct2Dの初期設定時、バックバッファの取得で使用します。

---

## SetFont
- **指定したフォントデータを使用してフォント設定を適用します。**

##### パラメータ
- `FontData data`: フォントの名前、サイズ、スタイルなどを指定したものです。

---

## DrawString
- **指定した文字列を、指定された位置に描画します。**

##### パラメータ
- `string str`: 描画するテキスト。
- `D3DXVECTOR2 pos`: 描画ポジション。先頭文字の開始位置を指定します。
- `D2D1_DRAW_TEXT_OPTIONS options`: テキストの整形を指定します。
- `bool shadow`: 影の描画を指定します。

##### 使用例
``` cpp
// 描画
Write->DrawString("テスト", D3DXVECTOR2(90, 90), D2D1_DRAW_TEXT_OPTIONS_NONE);
```

---

## GetFontName
- **読み込んだフォントファイルから、フォント名を返します。**
- フォント設定時のフォント名の記載ミスが起きないため、おすすめです。

##### パラメータ
- `int num`: `FontList::Fontpath`で指定したフォントファイルの順番を指定。

##### 戻り値
- `wstring`: 指定されたフォントファイルのフォント名。

##### 使用例
``` cpp
// フォントを変更
data.font = Write->GetFontName(3);
```

---

## GetFontNameNum
- **読み込んだフォントファイルの数を返します。**

##### 戻り値
- `int`: 読み込んだフォントファイルの数。
---

## GetFontFamilyName
- **フォントファイルの名前を読み込み直します。**
- ロケール名を指定できるので、日本語版フォントを読み込みたい場合などに使用してください。

##### パラメータ
- `IDWriteFontCollection* customFontCollection`: フォントファミリー名を読み込み直すフォントコレクション。
- `const WCHAR* locale`: ロケール名。デフォルトでは`"en-us"`が選択されています。

##### 使用例
``` cpp
// 日本語のフォント名を取得
result = GetFontFamilyName(fontCollection.Get(), L"ja-JP");
```

---

## GetAllFontFamilyName
- **全てのフォント名を読み込み直します。**

##### 注意
英語や日本語のフォント名が全て読み込まれるので、

順番がフォントファイルの順番と異なることになります(`GetFontName`の使用が難しくなります)。

正直、あまりおすすめはしません。

##### パラメータ
- `IDWriteFontCollection* customFontCollection`: フォントファミリー名を読み込み直すフォントコレクションを指定します。

---

## FontLoader
- **フォントファイルを読み込み直します。**
- `FontList::FontPass`にパスを追加した際などにお使いください。
