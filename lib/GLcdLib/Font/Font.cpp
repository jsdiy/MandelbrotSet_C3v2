//	フォント
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Font.hpp"
#include "DefineFont5x7.hpp"

//初期化
void	Font::Initialize()
{
	dotX = Font5x7Info::DotX;
	dotY = Font5x7Info::DotY;
	gapX = Font5x7Info::GapX;
	gapY = Font5x7Info::GapY;
}

//文字の形を定義した配列を取得する
const	uint8_t*	Font::GetGlyph(char cc)
{
	uint8_t	cidx = static_cast<uint8_t>(cc);
	if (0x20 <= cidx && cidx <= 0x7F) { return Font5x7Info::DataChar[cidx - 0x20]; }
	if (0xA1 <= cidx && cidx <= 0xDF) { return Font5x7Info::KanaChar[cidx - 0xA1]; }
	return Font5x7Info::DataChar[0];	//スペース文字(0x20:' ')
}

//セグメント形式でフォントデータを取得する
const	uint8_t*	Font::GetFontDataAsSegmentFormat(char cc)
{
	const uint8_t* addr = GetGlyph(cc);
	for (uint8_t i = 0; i < dotX; i++) { fontDataBuf[i] = addr[i]; }
	return fontDataBuf;
}

//標準形式でフォントデータを取得する
/*
プログラム領域に格納されているのは↓この形式（セグメント形式）

'&'	= { 0x36, 0x49, 0x55, 0x22, 0x50 }
□□■■□■■□	0x36
□■□□■□□■	0x49
□■□■□■□■	0x55
□□■□□□■□	0x22
□■□■□□□□	0x50

↑これを、↓こうする。

'&'	= { 0x60, 0x90, 0xA0, 0x40, 0xA8, 0x90, 0x68, 0x00 }
□■■□□□□□	0x60
■□□■□□□□	0x90
■□■□□□□□	0xA0
□■□□□□□□	0x40
■□■□■□□□	0xA8
■□□■□□□□	0x90
□■■□■□□□	0x68
□□□□□□□□	0x00
*/
const	uint8_t*	Font::GetFontData(char cc)
{
	GetFontDataAsSegmentFormat(cc);	//fontDataBuf[]にセグメント形式のフォントデータが格納される

	uint8_t segDatas[dotX];
	for (uint8_t i = 0; i < dotX; i++) { segDatas[i] = fontDataBuf[i]; }	//コピー
	
	for (uint8_t i = 0; i < dotY + gapY; i++)
	{
		uint8_t bitMask = (1 << i);
		fontDataBuf[i] = 0x00;
		for (uint8_t j = 0; j < dotX; j++)
		{
			uint8_t dot = (segDatas[j] & bitMask) ? 1 : 0;
			fontDataBuf[i] |= (dot << (dotY - j));
		}
	}
	return fontDataBuf;
}
