#pragma once

#include <string>						// 文字列
#include <vector>						// 動的配列
#include <wrl.h>						// ComPtr
#include <memory>

#pragma warning(push)
#pragma warning(disable:4005)

#include <d2d1.h>						// Direct2D
#include <DWrite.h>						// DirectWrite

#pragma warning(pop)

#pragma comment(lib,"d2d1.lib")			// Direct2D用
#pragma comment(lib,"Dwrite.lib")		// DirectWrite用

using namespace Microsoft;

class CustomFontCollectionLoader;

//=============================================================================
//		フォントの保存場所
//=============================================================================
namespace FontList
{
	const std::wstring FontPath[] =
	{
		L"font\\Sample.ttf",
		L"font\\サンプル.ttf",
	};
}

//=============================================================================
//		フォント設定
//=============================================================================
struct FontData
{
	std::wstring font;							// フォント名
	IDWriteFontCollection* fontCollection;		// フォントコレクション
	DWRITE_FONT_WEIGHT fontWeight;				// フォントの太さ
	DWRITE_FONT_STYLE fontStyle;				// フォントスタイル
	DWRITE_FONT_STRETCH fontStretch;			// フォントの幅
	FLOAT fontSize;								// フォントサイズ
	WCHAR const* localeName;					// ロケール名
	DWRITE_TEXT_ALIGNMENT textAlignment;		// テキストの配置
	D2D1_COLOR_F Color;							// フォントの色

	// デフォルト設定
	FontData()
	{
		font = L"";
		fontCollection = nullptr;
		fontWeight = DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL;
		fontStyle = DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL;
		fontStretch = DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL;
		fontSize = 20;
		localeName = L"ja-jp";
		textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
		Color = D2D1::ColorF(D2D1::ColorF::White);
	}
};

//=============================================================================
//		DirectWrite
//=============================================================================
class DirectWriteCustomFont
{
public:

	// デフォルトコンストラクタを制限
	DirectWriteCustomFont() = delete;

	// コンストラクタ
	// 第1引数：フォント設定
	DirectWriteCustomFont(FontData* set) :Setting(*set) {};

	// 初期化関数
	HRESULT Init(IDXGISwapChain* swapChain);

	// フォント設定
	// 第1引数：フォントデータ構造体
	HRESULT SetFont(FontData set);

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
	HRESULT SetFont
	(
		WCHAR const*			fontname,			// フォント名
		DWRITE_FONT_WEIGHT		fontWeight,			// フォントの太さ
		DWRITE_FONT_STYLE		fontStyle,			// フォントスタイル
		DWRITE_FONT_STRETCH		fontStretch,		// フォントの幅
		FLOAT					fontSize,			// フォントサイズ
		WCHAR const*			localeName,			// ロケール名
		DWRITE_TEXT_ALIGNMENT	textAlignment,		// テキストの配置
		D2D1_COLOR_F			Color				// フォントの色
	);

	// フォント設定
	// 第1引数：フォント// 第1引数：フォントリストのナンバー
	// 第2引数：フォントコレクション（nullptr）
	// 第3引数：フォントの太さ（DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD等）
	// 第4引数：フォントスタイル（DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC）
	// 第5引数：フォントの幅（DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED等）
	// 第6引数：フォントサイズ（20, 30等）
	// 第7引数：ロケール名（L"ja-jp"等）
	// 第8引数：テキストの配置（DWRITE_TEXT_ALIGNMENT_LEADING：前, 等）
	// 第9引数：フォントの色（D2D1::ColorF(D2D1::ColorF::Black)：黒, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))：RGBA指定等）
	HRESULT SetFont
	(
		int num,									// フォント名
		DWRITE_FONT_WEIGHT		fontWeight,			// フォントの太さ
		DWRITE_FONT_STYLE		fontStyle,			// フォントスタイル
		DWRITE_FONT_STRETCH		fontStretch,		// フォントの幅
		FLOAT					fontSize,			// フォントサイズ
		WCHAR const* localeName,					// ロケール名
		DWRITE_TEXT_ALIGNMENT	textAlignment,		// テキストの配置
		D2D1_COLOR_F			Color				// フォントの色
	);

	// 文字描画
	// string：文字列
	// pos：描画ポジション
	// options：テキストの整形
	HRESULT DrawString(std::string str, D3DXVECTOR2 pos, D2D1_DRAW_TEXT_OPTIONS options);

	// 文字描画
	// string：文字列
	// rect：領域指定
	// options：テキストの整形
	HRESULT DrawString(std::string str, D2D1_RECT_F rect, D2D1_DRAW_TEXT_OPTIONS options);

	// 指定されたパスのフォントを読み込む
	HRESULT FontLoader();

	// フォント名を取得する
	std::wstring GetFontName(int num);

	// 読み込んだフォント名の数を取得する
	int GetFontNameNum();

	// フォント名を取得し直す
	HRESULT GetFontFamilyName(IDWriteFontCollection* customFontCollection, const WCHAR* locale = L"en-us");

	// 全てのフォント名を取得し直す
	HRESULT GetAllFontFamilyName(IDWriteFontCollection* customFontCollection);

	// カスタムフォントコレクション
	WRL::ComPtr <IDWriteFontCollection> fontCollection = nullptr;

private:

	WRL::ComPtr <ID2D1Factory>			pD2DFactory = nullptr;		// Direct2Dリソース
	WRL::ComPtr <ID2D1RenderTarget>		pRenderTarget = nullptr;	// Direct2Dレンダーターゲット
	WRL::ComPtr <ID2D1SolidColorBrush>	pBrush = nullptr;			// Direct2Dブラシ設定
	WRL::ComPtr <IDWriteFactory>		pDWriteFactory = nullptr;	// DirectWriteリソース
	WRL::ComPtr <IDWriteTextFormat>		pTextFormat = nullptr;		// DirectWriteテキスト形式
	WRL::ComPtr <IDWriteTextLayout>		pTextLayout = nullptr;		// DirectWriteテキスト書式
	WRL::ComPtr <IDXGISurface>			pBackBuffer = nullptr;		// サーフェス情報

	// フォントファイルリスト
	std::vector<WRL::ComPtr<IDWriteFontFile>> pFontFileList;

	// フォントデータ
	FontData  Setting = FontData();

	// フォント名リスト
	std::vector<std::wstring> fontNamesList;

	// フォントのファイル名を取得する
	WCHAR* GetFontFileNameWithoutExtension(const std::wstring& filePath);

	// stringをwstringへ変換する
	std::wstring StringToWString(std::string oString);
};

