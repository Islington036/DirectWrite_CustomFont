#pragma once

#include "main.h"

#include "DirectWriteCustomFont.h"

// フォントコレクションローダー
WRL::ComPtr <CustomFontCollectionLoader> pFontCollectionLoader = nullptr;

//=============================================================================
//		カスタムファイルローダー
//=============================================================================
class CustomFontFileEnumerator : public IDWriteFontFileEnumerator 
{
public:
	CustomFontFileEnumerator(IDWriteFactory* factory, const std::vector<std::wstring>& fontFilePaths)
		: refCount_(0), factory_(factory), fontFilePaths_(fontFilePaths), currentFileIndex_(-1) 
	{
		factory_->AddRef();
	}

	~CustomFontFileEnumerator()
	{
		factory_->Release();
	}

	IFACEMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override
	{
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteFontCollectionLoader))
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else
		{
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	}

	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&refCount_);
	}

	IFACEMETHODIMP_(ULONG) Release() override
	{
		ULONG newCount = InterlockedDecrement(&refCount_);
		if (newCount == 0)
			delete this;

		return newCount;
	}

	IFACEMETHODIMP MoveNext(OUT BOOL* hasCurrentFile) override {
		if (++currentFileIndex_ < static_cast<int>(fontFilePaths_.size())) {
			*hasCurrentFile = TRUE;
			return S_OK;
		}
		else {
			*hasCurrentFile = FALSE;
			return S_OK;
		}
	}

	IFACEMETHODIMP GetCurrentFontFile(OUT IDWriteFontFile** fontFile) override 
	{
		// フォントファイルを読み込む
		return factory_->CreateFontFileReference(fontFilePaths_[currentFileIndex_].c_str(), nullptr, fontFile);
	}

private:
	ULONG refCount_;

	// DirectWriteファクトリ
	IDWriteFactory* factory_;

	// フォントファイルのパス
	std::vector<std::wstring> fontFilePaths_;

	// 現在のファイルインデックス
	int currentFileIndex_;
};

//=============================================================================
//		カスタムフォントコレクションローダー
//=============================================================================
class CustomFontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
	// コンストラクタ
	CustomFontCollectionLoader() : refCount_(0) {}

	// IUnknown メソッド
	IFACEMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override
	{
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteFontCollectionLoader))
		{
			*ppvObject = this;
			AddRef();
			return S_OK;
		}
		else
		{
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}
	}

	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&refCount_);
	}

	IFACEMETHODIMP_(ULONG) Release() override
	{
		ULONG newCount = InterlockedDecrement(&refCount_);
		if (newCount == 0)
			delete this;

		return newCount;
	}

	// IDWriteFontCollectionLoader メソッド
	IFACEMETHODIMP CreateEnumeratorFromKey
	(
		IDWriteFactory* factory,
		void const* collectionKey,
		UINT32 collectionKeySize,
		OUT IDWriteFontFileEnumerator** fontFileEnumerator) override
	{
		// 読み込むフォントファイルのパスを渡す
		std::vector<std::wstring> fontFilePaths(std::begin(FontList::FontPass), std::end(FontList::FontPass));

		// カスタムフォントファイル列挙子の作成
		*fontFileEnumerator = new (std::nothrow) CustomFontFileEnumerator(factory, fontFilePaths);

		// メモリ不足の場合はエラーを返す
		if (*fontFileEnumerator == nullptr) { return E_OUTOFMEMORY; }

		return S_OK;
	}

private:
	ULONG refCount_;
};

// 初期化処理
HRESULT DirectWriteCustomFont::Init(IDXGISwapChain* swapChain)
{
	// 処理の結果
	HRESULT result = S_OK;

	// Direct2Dファクトリ情報の初期化
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory.GetAddressOf());
	if (FAILED(result)) { return result; }

	// バックバッファの取得
	result = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(result)) { return result; }

	// dpiの設定
	FLOAT dpiX;
	FLOAT dpiY;
	pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

	// レンダーターゲットの作成
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);
	
	// サーフェスに描画するレンダーターゲットを作成
	result = pD2DFactory->CreateDxgiSurfaceRenderTarget(pBackBuffer.Get(), &props, pRenderTarget.GetAddressOf());
	if (FAILED(result)) { return result; }

	// アンチエイリアシングモードの設定
	pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	// IDWriteFactoryの作成
	result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(pDWriteFactory.GetAddressOf()));
	if (FAILED(result)) { return result; }

	// カスタムフォントコレクションローダー
	pFontCollectionLoader = new CustomFontCollectionLoader();

	// カスタムフォントコレクションローダーの作成
	result = pDWriteFactory->RegisterFontCollectionLoader(pFontCollectionLoader.Get());
	if (FAILED(result)) { return result; }

	// フォントファイルの読み込み
	result = FontLoader();
	if (FAILED(result)) { return result; }

	// フォントを設定
	result = SetFont(Setting);
	if (FAILED(result)) { return result; }

	return result;
}

// 指定されたパスのフォントを読み込む
HRESULT DirectWriteCustomFont::FontLoader()
{
	// 処理の結果
	HRESULT result = S_OK;

	// カスタムフォントコレクションの作成
	result = pDWriteFactory->CreateCustomFontCollection
	(
		pFontCollectionLoader.Get(),
		pFontFileList.data(),
		pFontFileList.size(),
		&fontCollection
	);
	if (FAILED(result)) { return result; }

	// フォント名を取得
	result = GetFontFamilyName(fontCollection.Get());
	if (FAILED(result)) { return result; }

	return S_OK;
}

// フォント名を取得する
std::wstring DirectWriteCustomFont::GetFontName(int num)
{
	// フォント名のリストが空だった場合
	if (fontNamesList.empty())
	{
		return nullptr;
	}

	// リストのサイズを超えていた場合
	if (num >= static_cast<int>(fontNamesList.size()))
	{
		return fontNamesList[0];
	}

	return fontNamesList[num];
}

// 読み込んだフォント名の数を取得する
int DirectWriteCustomFont::GetFontNameNum()
{
	return fontNamesList.size();
}

// フォント設定
// 第1引数：フォントデータ構造体
HRESULT DirectWriteCustomFont::SetFont(FontData set)
{
	// 処理の結果
	HRESULT result = S_OK;

	// 設定をコピー
	Setting = set;

	//関数CreateTextFormat()
	//第1引数：フォント名（L"メイリオ", L"Arial", L"Meiryo UI"等）
	//第2引数：フォントコレクション（nullptr）
	//第3引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
	//第4引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
	//第5引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
	//第6引数：フォントサイズ（20, 30等）
	//第7引数：ロケール名（L""）
	//第8引数：テキストフォーマット（&g_pTextFormat）
	result = pDWriteFactory->CreateTextFormat
	(
		GetFontFileNameWithoutExtension(Setting.font.c_str()),
		fontCollection.Get(),
		Setting.fontWeight,
		Setting.fontStyle,
		Setting.fontStretch,
		Setting.fontSize,
		Setting.localeName,
		pTextFormat.GetAddressOf()
	);
	if (FAILED(result)) { return result; }

	//関数SetTextAlignment()
	//第1引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, DWRITE_TEXT_ALIGNMENT_TRAILING：後, DWRITE_TEXT_ALIGNMENT_CENTER：中央,
	//                         DWRITE_TEXT_ALIGNMENT_JUSTIFIED：行いっぱい）
	result = pTextFormat->SetTextAlignment(Setting.textAlignment);
	if (FAILED(result)) { return result; }

	//関数CreateSolidColorBrush()
	//第1引数：フォント色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定）
	result = pRenderTarget->CreateSolidColorBrush(Setting.Color, pBrush.GetAddressOf());
	if (FAILED(result)) { return result; }

	return result;
}

//=================================================================================================================================
// フォント設定
// 第1引数：フォント名（L"メイリオ", L"Arial", L"Meiryo UI"等）
// 第2引数：フォントコレクション（nullptr）
// 第3引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
// 第4引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
// 第5引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
// 第6引数：フォントサイズ（20, 30等）
// 第7引数：ロケール名（L"ja-jp"等）
// 第8引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, 等）
// 第9引数：フォントの色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定等）
//=================================================================================================================================
HRESULT DirectWriteCustomFont::SetFont(WCHAR const* fontname, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle,
									DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName,
									DWRITE_TEXT_ALIGNMENT textAlignment, D2D1_COLOR_F Color)
{
	// 処理の結果
	HRESULT result = S_OK;

	pDWriteFactory->CreateTextFormat(GetFontFileNameWithoutExtension(fontname), fontCollection.Get(), fontWeight, fontStyle, fontStretch, fontSize, localeName, &pTextFormat);
	if (FAILED(result)) { return result; }

	pTextFormat->SetTextAlignment(textAlignment);
	if (FAILED(result)) { return result; }

	pRenderTarget->CreateSolidColorBrush(Color, pBrush.GetAddressOf());
	if (FAILED(result)) { return result; }

	return result;
}

//=================================================================================================================================
// フォント設定
// 第1引数：フォントリストのナンバー
// 第2引数：フォントコレクション（nullptr）
// 第3引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
// 第4引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
// 第5引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
// 第6引数：フォントサイズ（20, 30等）
// 第7引数：ロケール名（L"ja-jp"等）
// 第8引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, 等）
// 第9引数：フォントの色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定等）
//=================================================================================================================================
HRESULT DirectWriteCustomFont::SetFont(int num, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, 
									DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName,
									DWRITE_TEXT_ALIGNMENT textAlignment, D2D1_COLOR_F Color)
{
	// 処理の結果
	HRESULT result = S_OK;

	pDWriteFactory->CreateTextFormat(GetFontName(num).c_str(), fontCollection.Get(), fontWeight, fontStyle, fontStretch, fontSize, localeName, &pTextFormat);
	if (FAILED(result)) { return result; }

	pTextFormat->SetTextAlignment(textAlignment);
	if (FAILED(result)) { return result; }

	pRenderTarget->CreateSolidColorBrush(Color, pBrush.GetAddressOf());
	if (FAILED(result)) { return result; }

	return result;
}

//====================================
// 文字描画
// string：文字列
// pos：描画ポジション
// options：テキストの整形
//====================================
HRESULT DirectWriteCustomFont::DrawString(std::string str, D3DXVECTOR2 pos, D2D1_DRAW_TEXT_OPTIONS options)
{
	// 処理の結果
	HRESULT result = S_OK;

	// 文字列の変換
	std::wstring wstr = StringToWString(str.c_str());

	// ターゲットサイズの取得
	D2D1_SIZE_F TargetSize = pRenderTarget->GetSize();

	// テキストレイアウトを作成
	result = pDWriteFactory->CreateTextLayout(wstr.c_str(), wstr.size(), pTextFormat.Get(), TargetSize.width, TargetSize.height, pTextLayout.GetAddressOf());
	if (FAILED(result)) { return result; }

	// 描画位置の確定
	D2D1_POINT_2F pounts;
	pounts.x = pos.x;
	pounts.y = pos.y;

	// 描画の開始
	pRenderTarget->BeginDraw();
	
	// 描画処理
	pRenderTarget->DrawTextLayout(pounts, pTextLayout.Get(), pBrush.Get(), options);
	
	// 描画の終了
	result = pRenderTarget->EndDraw();
	if (FAILED(result)) { return result; }

	return S_OK;
}

//====================================
// 文字描画
// string：文字列
// rect：領域指定
// options：テキストの整形
	//====================================
HRESULT DirectWriteCustomFont::DrawString(std::string str, D2D1_RECT_F rect, D2D1_DRAW_TEXT_OPTIONS options)
{
	// 処理の結果
	HRESULT result = S_OK;

	// 文字列の変換
	std::wstring wstr = StringToWString(str.c_str());

	// 描画の開始
	pRenderTarget->BeginDraw();

	// 描画処理
	pRenderTarget->DrawText(wstr.c_str(), wstr.size(), pTextFormat.Get(), rect, pBrush.Get(), options);

	// 描画の終了
	result = pRenderTarget->EndDraw();
	if (FAILED(result)) { return result; }


	return S_OK;
}

// フォント名を取得
HRESULT DirectWriteCustomFont::GetFontFamilyName(IDWriteFontCollection* customFontCollection, const WCHAR* locale)
{
	// 処理の結果
	HRESULT result = S_OK;

	// フォントファミリー名一覧をリセット
	std::vector<std::wstring>().swap(fontNamesList);

	// フォントの数を取得
	UINT32 familyCount = customFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// フォントファミリーの取得
		WRL::ComPtr <IDWriteFontFamily> fontFamily = nullptr;
		result = customFontCollection->GetFontFamily(i, fontFamily.GetAddressOf());
		if (FAILED(result)) { return result; }

		// フォントファミリー名の一覧を取得
		WRL::ComPtr <IDWriteLocalizedStrings> familyNames = nullptr;
		result = fontFamily->GetFamilyNames(familyNames.GetAddressOf());
		if (FAILED(result)) { return result; }

		// 指定されたロケールに対応するインデックスを検索
		UINT32 index = 0;
		BOOL exists = FALSE;
		result = familyNames->FindLocaleName(locale, &index, &exists);
		if (FAILED(result)) { return result; }

		// 指定されたロケールが見つからなかった場合は、デフォルトのロケールを使用
		if (!exists)
		{
			result = familyNames->FindLocaleName(L"en-us", &index, &exists);
			if (FAILED(result)) { return result; }
		}

		// フォントファミリー名の長さを取得
		UINT32 length = 0;
		result = familyNames->GetStringLength(index, &length);
		if (FAILED(result)) { return result; }

		// フォントファミリー名の取得
		WCHAR* name = new WCHAR[length + 1];
		result = familyNames->GetString(index, name, length + 1);
		if (FAILED(result)) { return result; }

		// フォントファミリー名を追加
		fontNamesList.push_back(name);

		// フォントファミリー名の破棄
		delete[] name;
	}

	return result;
}

// 全てのフォント名を取得
HRESULT DirectWriteCustomFont::GetAllFontFamilyName(IDWriteFontCollection* customFontCollection)
{
	// 処理の結果
	HRESULT result = S_OK;

	// フォントファミリー名一覧をリセット
	std::vector<std::wstring>().swap(fontNamesList);

	// フォントファミリーの数を取得
	UINT32 familyCount = customFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// フォントファミリーの取得
		WRL::ComPtr <IDWriteFontFamily> fontFamily = nullptr;
		result = customFontCollection->GetFontFamily(i, fontFamily.GetAddressOf());
		if (FAILED(result)) { return result; }

		// フォントファミリー名の一覧を取得
		WRL::ComPtr <IDWriteLocalizedStrings> familyNames = nullptr;
		result = fontFamily->GetFamilyNames(familyNames.GetAddressOf());
		if (FAILED(result)) { return result; }

		// フォントファミリー名の数を取得
		UINT32 nameCount = familyNames->GetCount();

		// フォントファミリー名の数だけ繰り返す
		for (UINT32 j = 0; j < nameCount; ++j)
		{
			// フォントファミリー名の長さを取得
			UINT32 length = 0;
			result = familyNames->GetStringLength(j, &length);
			if (FAILED(result)) { return result; }

			// フォントファミリー名の取得
			WCHAR* name = new WCHAR[length + 1];
			result = familyNames->GetString(j, name, length + 1);
			if (FAILED(result)) { return result; }

			// フォントファミリー名を追加
			fontNamesList.push_back(name);

			// フォントファミリー名の破棄
			delete[] name;
		}
	}

	return result;
}

// 拡張子を除くファイル名を返す
WCHAR* DirectWriteCustomFont::GetFontFileNameWithoutExtension(const std::wstring& filePath)
{
	// 末尾から検索してファイル名と拡張子の位置を取得
	size_t start = filePath.find_last_of(L"/\\") + 1;
	size_t end = filePath.find_last_of(L'.');

	// ファイル名を取得
    std::wstring fileNameWithoutExtension = filePath.substr(start, end - start).c_str();

	// 新しいWCHAR配列を作成
	WCHAR* fileName = new WCHAR[fileNameWithoutExtension.length() + 1];

	// 文字列をコピー
	wcscpy_s(fileName, fileNameWithoutExtension.length() + 1, fileNameWithoutExtension.c_str());

	// ファイル名を返す
	return fileName;
}

// stringをwstringへ変換する
std::wstring DirectWriteCustomFont::StringToWString(std::string oString)
{
	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);

	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);

	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// バッファの破棄
	delete[] cpUCS2;

	// 変換結果を返す
	return(oRet);
}