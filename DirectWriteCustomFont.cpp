#pragma once

#include "main.h"

#include "DirectWriteCustomFont.h"

// �t�H���g�R���N�V�������[�_�[
WRL::ComPtr <CustomFontCollectionLoader> pFontCollectionLoader = nullptr;

//=============================================================================
//		�J�X�^���t�@�C�����[�_�[
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
		// �t�H���g�t�@�C����ǂݍ���
		return factory_->CreateFontFileReference(fontFilePaths_[currentFileIndex_].c_str(), nullptr, fontFile);
	}

private:
	ULONG refCount_;

	// DirectWrite�t�@�N�g��
	IDWriteFactory* factory_;

	// �t�H���g�t�@�C���̃p�X
	std::vector<std::wstring> fontFilePaths_;

	// ���݂̃t�@�C���C���f�b�N�X
	int currentFileIndex_;
};

//=============================================================================
//		�J�X�^���t�H���g�R���N�V�������[�_�[
//=============================================================================
class CustomFontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
	// �R���X�g���N�^
	CustomFontCollectionLoader() : refCount_(0) {}

	// IUnknown ���\�b�h
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

	// IDWriteFontCollectionLoader ���\�b�h
	IFACEMETHODIMP CreateEnumeratorFromKey
	(
		IDWriteFactory* factory,
		void const* collectionKey,
		UINT32 collectionKeySize,
		OUT IDWriteFontFileEnumerator** fontFileEnumerator) override
	{
		// �ǂݍ��ރt�H���g�t�@�C���̃p�X��n��
		std::vector<std::wstring> fontFilePaths(std::begin(FontList::FontPass), std::end(FontList::FontPass));

		// �J�X�^���t�H���g�t�@�C���񋓎q�̍쐬
		*fontFileEnumerator = new (std::nothrow) CustomFontFileEnumerator(factory, fontFilePaths);

		// �������s���̏ꍇ�̓G���[��Ԃ�
		if (*fontFileEnumerator == nullptr) { return E_OUTOFMEMORY; }

		return S_OK;
	}

private:
	ULONG refCount_;
};

// ����������
HRESULT DirectWriteCustomFont::Init(IDXGISwapChain* swapChain)
{
	// �����̌���
	HRESULT result = S_OK;

	// Direct2D�t�@�N�g�����̏�����
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory.GetAddressOf());
	if (FAILED(result)) { return result; }

	// �o�b�N�o�b�t�@�̎擾
	result = swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(result)) { return result; }

	// dpi�̐ݒ�
	FLOAT dpiX;
	FLOAT dpiY;
	pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

	// �����_�[�^�[�Q�b�g�̍쐬
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);
	
	// �T�[�t�F�X�ɕ`�悷�郌���_�[�^�[�Q�b�g���쐬
	result = pD2DFactory->CreateDxgiSurfaceRenderTarget(pBackBuffer.Get(), &props, pRenderTarget.GetAddressOf());
	if (FAILED(result)) { return result; }

	// �A���`�G�C���A�V���O���[�h�̐ݒ�
	pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	// IDWriteFactory�̍쐬
	result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(pDWriteFactory.GetAddressOf()));
	if (FAILED(result)) { return result; }

	// �J�X�^���t�H���g�R���N�V�������[�_�[
	pFontCollectionLoader = new CustomFontCollectionLoader();

	// �J�X�^���t�H���g�R���N�V�������[�_�[�̍쐬
	result = pDWriteFactory->RegisterFontCollectionLoader(pFontCollectionLoader.Get());
	if (FAILED(result)) { return result; }

	// �t�H���g�t�@�C���̓ǂݍ���
	result = FontLoader();
	if (FAILED(result)) { return result; }

	// �t�H���g��ݒ�
	result = SetFont(Setting);
	if (FAILED(result)) { return result; }

	return result;
}

// �w�肳�ꂽ�p�X�̃t�H���g��ǂݍ���
HRESULT DirectWriteCustomFont::FontLoader()
{
	// �����̌���
	HRESULT result = S_OK;

	// �J�X�^���t�H���g�R���N�V�����̍쐬
	result = pDWriteFactory->CreateCustomFontCollection
	(
		pFontCollectionLoader.Get(),
		pFontFileList.data(),
		pFontFileList.size(),
		&fontCollection
	);
	if (FAILED(result)) { return result; }

	// �t�H���g�����擾
	result = GetFontFamilyName(fontCollection.Get());
	if (FAILED(result)) { return result; }

	return S_OK;
}

// �t�H���g�����擾����
std::wstring DirectWriteCustomFont::GetFontName(int num)
{
	// �t�H���g���̃��X�g���󂾂����ꍇ
	if (fontNamesList.empty())
	{
		return nullptr;
	}

	// ���X�g�̃T�C�Y�𒴂��Ă����ꍇ
	if (num >= static_cast<int>(fontNamesList.size()))
	{
		return fontNamesList[0];
	}

	return fontNamesList[num];
}

// �ǂݍ��񂾃t�H���g���̐����擾����
int DirectWriteCustomFont::GetFontNameNum()
{
	return fontNamesList.size();
}

// �t�H���g�ݒ�
// ��1�����F�t�H���g�f�[�^�\����
HRESULT DirectWriteCustomFont::SetFont(FontData set)
{
	// �����̌���
	HRESULT result = S_OK;

	// �ݒ���R�s�[
	Setting = set;

	//�֐�CreateTextFormat()
	//��1�����F�t�H���g���iL"���C���I", L"Arial", L"Meiryo UI"���j
	//��2�����F�t�H���g�R���N�V�����inullptr�j
	//��3�����F�t�H���g�̑����iDWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD���j
	//��4�����F�t�H���g�X�^�C���iDWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC�j
	//��5�����F�t�H���g�̕��iDWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED���j
	//��6�����F�t�H���g�T�C�Y�i20, 30���j
	//��7�����F���P�[�����iL""�j
	//��8�����F�e�L�X�g�t�H�[�}�b�g�i&g_pTextFormat�j
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

	//�֐�SetTextAlignment()
	//��1�����F�e�L�X�g�̔z�u�iDWRITE_TEXT_ALIGNMENT_LEADING�F�O, DWRITE_TEXT_ALIGNMENT_TRAILING�F��, DWRITE_TEXT_ALIGNMENT_CENTER�F����,
	//                         DWRITE_TEXT_ALIGNMENT_JUSTIFIED�F�s�����ς��j
	result = pTextFormat->SetTextAlignment(Setting.textAlignment);
	if (FAILED(result)) { return result; }

	//�֐�CreateSolidColorBrush()
	//��1�����F�t�H���g�F�iD2D1::ColorF(D2D1::ColorF::Black)�F��, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))�FRGBA�w��j
	result = pRenderTarget->CreateSolidColorBrush(Setting.Color, pBrush.GetAddressOf());
	if (FAILED(result)) { return result; }

	return result;
}

//=================================================================================================================================
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
//=================================================================================================================================
HRESULT DirectWriteCustomFont::SetFont(WCHAR const* fontname, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle,
									DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName,
									DWRITE_TEXT_ALIGNMENT textAlignment, D2D1_COLOR_F Color)
{
	// �����̌���
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
// �t�H���g�ݒ�
// ��1�����F�t�H���g���X�g�̃i���o�[
// ��2�����F�t�H���g�R���N�V�����inullptr�j
// ��3�����F�t�H���g�̑����iDWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_BOLD���j
// ��4�����F�t�H���g�X�^�C���iDWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STYLE_ITALIC�j
// ��5�����F�t�H���g�̕��iDWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STRETCH_EXTRA_EXPANDED���j
// ��6�����F�t�H���g�T�C�Y�i20, 30���j
// ��7�����F���P�[�����iL"ja-jp"���j
// ��8�����F�e�L�X�g�̔z�u�iDWRITE_TEXT_ALIGNMENT_LEADING�F�O, ���j
// ��9�����F�t�H���g�̐F�iD2D1::ColorF(D2D1::ColorF::Black)�F��, D2D1::ColorF(D2D1::ColorF(0.0f, 0.2f, 0.9f, 1.0f))�FRGBA�w�蓙�j
//=================================================================================================================================
HRESULT DirectWriteCustomFont::SetFont(int num, DWRITE_FONT_WEIGHT fontWeight, DWRITE_FONT_STYLE fontStyle, 
									DWRITE_FONT_STRETCH fontStretch, FLOAT fontSize, WCHAR const* localeName,
									DWRITE_TEXT_ALIGNMENT textAlignment, D2D1_COLOR_F Color)
{
	// �����̌���
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
// �����`��
// string�F������
// pos�F�`��|�W�V����
// options�F�e�L�X�g�̐��`
//====================================
HRESULT DirectWriteCustomFont::DrawString(std::string str, D3DXVECTOR2 pos, D2D1_DRAW_TEXT_OPTIONS options)
{
	// �����̌���
	HRESULT result = S_OK;

	// ������̕ϊ�
	std::wstring wstr = StringToWString(str.c_str());

	// �^�[�Q�b�g�T�C�Y�̎擾
	D2D1_SIZE_F TargetSize = pRenderTarget->GetSize();

	// �e�L�X�g���C�A�E�g���쐬
	result = pDWriteFactory->CreateTextLayout(wstr.c_str(), wstr.size(), pTextFormat.Get(), TargetSize.width, TargetSize.height, pTextLayout.GetAddressOf());
	if (FAILED(result)) { return result; }

	// �`��ʒu�̊m��
	D2D1_POINT_2F pounts;
	pounts.x = pos.x;
	pounts.y = pos.y;

	// �`��̊J�n
	pRenderTarget->BeginDraw();
	
	// �`�揈��
	pRenderTarget->DrawTextLayout(pounts, pTextLayout.Get(), pBrush.Get(), options);
	
	// �`��̏I��
	result = pRenderTarget->EndDraw();
	if (FAILED(result)) { return result; }

	return S_OK;
}

//====================================
// �����`��
// string�F������
// rect�F�̈�w��
// options�F�e�L�X�g�̐��`
	//====================================
HRESULT DirectWriteCustomFont::DrawString(std::string str, D2D1_RECT_F rect, D2D1_DRAW_TEXT_OPTIONS options)
{
	// �����̌���
	HRESULT result = S_OK;

	// ������̕ϊ�
	std::wstring wstr = StringToWString(str.c_str());

	// �`��̊J�n
	pRenderTarget->BeginDraw();

	// �`�揈��
	pRenderTarget->DrawText(wstr.c_str(), wstr.size(), pTextFormat.Get(), rect, pBrush.Get(), options);

	// �`��̏I��
	result = pRenderTarget->EndDraw();
	if (FAILED(result)) { return result; }


	return S_OK;
}

// �t�H���g�����擾
HRESULT DirectWriteCustomFont::GetFontFamilyName(IDWriteFontCollection* customFontCollection, const WCHAR* locale)
{
	// �����̌���
	HRESULT result = S_OK;

	// �t�H���g�t�@�~���[���ꗗ�����Z�b�g
	std::vector<std::wstring>().swap(fontNamesList);

	// �t�H���g�̐����擾
	UINT32 familyCount = customFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// �t�H���g�t�@�~���[�̎擾
		WRL::ComPtr <IDWriteFontFamily> fontFamily = nullptr;
		result = customFontCollection->GetFontFamily(i, fontFamily.GetAddressOf());
		if (FAILED(result)) { return result; }

		// �t�H���g�t�@�~���[���̈ꗗ���擾
		WRL::ComPtr <IDWriteLocalizedStrings> familyNames = nullptr;
		result = fontFamily->GetFamilyNames(familyNames.GetAddressOf());
		if (FAILED(result)) { return result; }

		// �w�肳�ꂽ���P�[���ɑΉ�����C���f�b�N�X������
		UINT32 index = 0;
		BOOL exists = FALSE;
		result = familyNames->FindLocaleName(locale, &index, &exists);
		if (FAILED(result)) { return result; }

		// �w�肳�ꂽ���P�[����������Ȃ������ꍇ�́A�f�t�H���g�̃��P�[�����g�p
		if (!exists)
		{
			result = familyNames->FindLocaleName(L"en-us", &index, &exists);
			if (FAILED(result)) { return result; }
		}

		// �t�H���g�t�@�~���[���̒������擾
		UINT32 length = 0;
		result = familyNames->GetStringLength(index, &length);
		if (FAILED(result)) { return result; }

		// �t�H���g�t�@�~���[���̎擾
		WCHAR* name = new WCHAR[length + 1];
		result = familyNames->GetString(index, name, length + 1);
		if (FAILED(result)) { return result; }

		// �t�H���g�t�@�~���[����ǉ�
		fontNamesList.push_back(name);

		// �t�H���g�t�@�~���[���̔j��
		delete[] name;
	}

	return result;
}

// �S�Ẵt�H���g�����擾
HRESULT DirectWriteCustomFont::GetAllFontFamilyName(IDWriteFontCollection* customFontCollection)
{
	// �����̌���
	HRESULT result = S_OK;

	// �t�H���g�t�@�~���[���ꗗ�����Z�b�g
	std::vector<std::wstring>().swap(fontNamesList);

	// �t�H���g�t�@�~���[�̐����擾
	UINT32 familyCount = customFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// �t�H���g�t�@�~���[�̎擾
		WRL::ComPtr <IDWriteFontFamily> fontFamily = nullptr;
		result = customFontCollection->GetFontFamily(i, fontFamily.GetAddressOf());
		if (FAILED(result)) { return result; }

		// �t�H���g�t�@�~���[���̈ꗗ���擾
		WRL::ComPtr <IDWriteLocalizedStrings> familyNames = nullptr;
		result = fontFamily->GetFamilyNames(familyNames.GetAddressOf());
		if (FAILED(result)) { return result; }

		// �t�H���g�t�@�~���[���̐����擾
		UINT32 nameCount = familyNames->GetCount();

		// �t�H���g�t�@�~���[���̐������J��Ԃ�
		for (UINT32 j = 0; j < nameCount; ++j)
		{
			// �t�H���g�t�@�~���[���̒������擾
			UINT32 length = 0;
			result = familyNames->GetStringLength(j, &length);
			if (FAILED(result)) { return result; }

			// �t�H���g�t�@�~���[���̎擾
			WCHAR* name = new WCHAR[length + 1];
			result = familyNames->GetString(j, name, length + 1);
			if (FAILED(result)) { return result; }

			// �t�H���g�t�@�~���[����ǉ�
			fontNamesList.push_back(name);

			// �t�H���g�t�@�~���[���̔j��
			delete[] name;
		}
	}

	return result;
}

// �g���q�������t�@�C������Ԃ�
WCHAR* DirectWriteCustomFont::GetFontFileNameWithoutExtension(const std::wstring& filePath)
{
	// �������猟�����ăt�@�C�����Ɗg���q�̈ʒu���擾
	size_t start = filePath.find_last_of(L"/\\") + 1;
	size_t end = filePath.find_last_of(L'.');

	// �t�@�C�������擾
    std::wstring fileNameWithoutExtension = filePath.substr(start, end - start).c_str();

	// �V����WCHAR�z����쐬
	WCHAR* fileName = new WCHAR[fileNameWithoutExtension.length() + 1];

	// ��������R�s�[
	wcscpy_s(fileName, fileNameWithoutExtension.length() + 1, fileNameWithoutExtension.c_str());

	// �t�@�C������Ԃ�
	return fileName;
}

// string��wstring�֕ϊ�����
std::wstring DirectWriteCustomFont::StringToWString(std::string oString)
{
	// SJIS �� wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);

	// �o�b�t�@�̎擾
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS �� wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);

	// string�̐���
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// �o�b�t�@�̔j��
	delete[] cpUCS2;

	// �ϊ����ʂ�Ԃ�
	return(oRet);
}