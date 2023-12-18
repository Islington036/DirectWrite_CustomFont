// ���ۂɎg�p������ł̃w�b�_�[�͓K�X�ǉ����Ă�������
#include "main.h"
#include"renderer.h"

#include "DirectWriteCustomFont.h"
#include "DirectWriteSample.h"

// �t�H���g�f�[�^
FontData data;

// DirectWrite�`��N���X
DirectWriteCustomFont* Write;

int drawNum = 0;

void DirectWriteSample::Init()
{
	// DirectWrite�p�R���|�[�l���g���쐬
	Write = new DirectWriteCustomFont(&data);

	// ������
	Write->Init(Renderer::GetSwapChain());

	// �t�H���g�f�[�^������
	data.fontSize = 60;
	data.fontWeight = DWRITE_FONT_WEIGHT_ULTRA_BLACK;
	data.Color = D2D1::ColorF(D2D1::ColorF::Red);
	data.font = Write->GetFontName(3);
	data.shadowColor = D2D1::ColorF(D2D1::ColorF::White);
	data.shadowOffset = D2D1::Point2F(5.0f, -5.0f);

	// �t�H���g���Z�b�g
	Write->SetFont(data);
}

void DirectWriteSample::Uninit()
{
	// ���������
	delete Write;
	Write = nullptr;
}

void DirectWriteSample::Update()
{
}

void DirectWriteSample::Draw()
{
	Write->DrawString("�����̓e�X�g��ʂł�", D3DXVECTOR2(90, 90), D2D1_DRAW_TEXT_OPTIONS_NONE);

	Write->DrawString("�`��ʒu�͒������Ă�������", D3DXVECTOR2(90, 390), D2D1_DRAW_TEXT_OPTIONS_NONE);
}