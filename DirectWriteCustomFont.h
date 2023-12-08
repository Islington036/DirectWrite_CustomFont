#pragma once

#include <string>						// ������
#include <vector>						// ���I�z��
#include <wrl.h>						// ComPtr
#include <memory>

#pragma warning(push)
#pragma warning(disable:4005)

#include <d2d1.h>						// Direct2D
#include <DWrite.h>						// DirectWrite

#pragma warning(pop)

#pragma comment(lib,"d2d1.lib")			// Direct2D�p
#pragma comment(lib,"Dwrite.lib")		// DirectWrite�p

using namespace Microsoft;

class CustomFontCollectionLoader;

//=============================================================================
//		�t�H���g�̕ۑ��ꏊ
//=============================================================================
namespace FontList
{
	const std::wstring FontPath[] =
	{
		L"font\\Sample.ttf",
		L"font\\�T���v��.ttf",
	};
}

//=============================================================================
//		�t�H���g�ݒ�
//=============================================================================
struct FontData
{
	std::wstring font;							// �t�H���g��
	IDWriteFontCollection* fontCollection;		// �t�H���g�R���N�V����
	DWRITE_FONT_WEIGHT fontWeight;				// �t�H���g�̑���
	DWRITE_FONT_STYLE fontStyle;				// �t�H���g�X�^�C��
	DWRITE_FONT_STRETCH fontStretch;			// �t�H���g�̕�
	FLOAT fontSize;								// �t�H���g�T�C�Y
	WCHAR const* localeName;					// ���P�[����
	DWRITE_TEXT_ALIGNMENT textAlignment;		// �e�L�X�g�̔z�u
	D2D1_COLOR_F Color;							// �t�H���g�̐F

	// �f�t�H���g�ݒ�
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

	// �f�t�H���g�R���X�g���N�^�𐧌�
	DirectWriteCustomFont() = delete;

	// �R���X�g���N�^
	// ��1�����F�t�H���g�ݒ�
	DirectWriteCustomFont(FontData* set) :Setting(*set) {};

	// �������֐�
	HRESULT Init(IDXGISwapChain* swapChain);

	// �t�H���g�ݒ�
	// ��1�����F�t�H���g�f�[�^�\����
	HRESULT SetFont(FontData set);

	// �t�H���g�ݒ�
	// ��1�����F�t�H���g���iL"���C���I", L"Arial", L"Meiryo UI"���j
	// ��2�����F�t�H���g�R���N�V�����inullptr�j
	// ��3�����F�t�H���g�̑����iDWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD���j
	// ��4�����F�t�H���g�X�^�C���iDWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC�j
	// ��5�����F�t�H���g�̕��iDWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED���j
	// ��6�����F�t�H���g�T�C�Y�i20, 30���j
	// ��7�����F���P�[�����iL"ja-jp"���j
	// ��8�����F�e�L�X�g�̔z�u�iDWRITE_TEXT_ALIGNMENT_LEADING�F�O, ���j
	// ��9�����F�t�H���g�̐F�iD2D1::ColorF(D2D1::ColorF::Black)�F��, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))�FRGBA�w�蓙�j
	HRESULT SetFont
	(
		WCHAR const*			fontname,			// �t�H���g��
		DWRITE_FONT_WEIGHT		fontWeight,			// �t�H���g�̑���
		DWRITE_FONT_STYLE		fontStyle,			// �t�H���g�X�^�C��
		DWRITE_FONT_STRETCH		fontStretch,		// �t�H���g�̕�
		FLOAT					fontSize,			// �t�H���g�T�C�Y
		WCHAR const*			localeName,			// ���P�[����
		DWRITE_TEXT_ALIGNMENT	textAlignment,		// �e�L�X�g�̔z�u
		D2D1_COLOR_F			Color				// �t�H���g�̐F
	);

	// �t�H���g�ݒ�
	// ��1�����F�t�H���g// ��1�����F�t�H���g���X�g�̃i���o�[
	// ��2�����F�t�H���g�R���N�V�����inullptr�j
	// ��3�����F�t�H���g�̑����iDWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD���j
	// ��4�����F�t�H���g�X�^�C���iDWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC�j
	// ��5�����F�t�H���g�̕��iDWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED���j
	// ��6�����F�t�H���g�T�C�Y�i20, 30���j
	// ��7�����F���P�[�����iL"ja-jp"���j
	// ��8�����F�e�L�X�g�̔z�u�iDWRITE_TEXT_ALIGNMENT_LEADING�F�O, ���j
	// ��9�����F�t�H���g�̐F�iD2D1::ColorF(D2D1::ColorF::Black)�F��, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))�FRGBA�w�蓙�j
	HRESULT SetFont
	(
		int num,									// �t�H���g��
		DWRITE_FONT_WEIGHT		fontWeight,			// �t�H���g�̑���
		DWRITE_FONT_STYLE		fontStyle,			// �t�H���g�X�^�C��
		DWRITE_FONT_STRETCH		fontStretch,		// �t�H���g�̕�
		FLOAT					fontSize,			// �t�H���g�T�C�Y
		WCHAR const* localeName,					// ���P�[����
		DWRITE_TEXT_ALIGNMENT	textAlignment,		// �e�L�X�g�̔z�u
		D2D1_COLOR_F			Color				// �t�H���g�̐F
	);

	// �����`��
	// string�F������
	// pos�F�`��|�W�V����
	// options�F�e�L�X�g�̐��`
	HRESULT DrawString(std::string str, D3DXVECTOR2 pos, D2D1_DRAW_TEXT_OPTIONS options);

	// �����`��
	// string�F������
	// rect�F�̈�w��
	// options�F�e�L�X�g�̐��`
	HRESULT DrawString(std::string str, D2D1_RECT_F rect, D2D1_DRAW_TEXT_OPTIONS options);

	// �w�肳�ꂽ�p�X�̃t�H���g��ǂݍ���
	HRESULT FontLoader();

	// �t�H���g�����擾����
	std::wstring GetFontName(int num);

	// �ǂݍ��񂾃t�H���g���̐����擾����
	int GetFontNameNum();

	// �t�H���g�����擾������
	HRESULT GetFontFamilyName(IDWriteFontCollection* customFontCollection, const WCHAR* locale = L"en-us");

	// �S�Ẵt�H���g�����擾������
	HRESULT GetAllFontFamilyName(IDWriteFontCollection* customFontCollection);

	// �J�X�^���t�H���g�R���N�V����
	WRL::ComPtr <IDWriteFontCollection> fontCollection = nullptr;

private:

	WRL::ComPtr <ID2D1Factory>			pD2DFactory = nullptr;		// Direct2D���\�[�X
	WRL::ComPtr <ID2D1RenderTarget>		pRenderTarget = nullptr;	// Direct2D�����_�[�^�[�Q�b�g
	WRL::ComPtr <ID2D1SolidColorBrush>	pBrush = nullptr;			// Direct2D�u���V�ݒ�
	WRL::ComPtr <IDWriteFactory>		pDWriteFactory = nullptr;	// DirectWrite���\�[�X
	WRL::ComPtr <IDWriteTextFormat>		pTextFormat = nullptr;		// DirectWrite�e�L�X�g�`��
	WRL::ComPtr <IDWriteTextLayout>		pTextLayout = nullptr;		// DirectWrite�e�L�X�g����
	WRL::ComPtr <IDXGISurface>			pBackBuffer = nullptr;		// �T�[�t�F�X���

	// �t�H���g�t�@�C�����X�g
	std::vector<WRL::ComPtr<IDWriteFontFile>> pFontFileList;

	// �t�H���g�f�[�^
	FontData  Setting = FontData();

	// �t�H���g�����X�g
	std::vector<std::wstring> fontNamesList;

	// �t�H���g�̃t�@�C�������擾����
	WCHAR* GetFontFileNameWithoutExtension(const std::wstring& filePath);

	// string��wstring�֕ϊ�����
	std::wstring StringToWString(std::string oString);
};

