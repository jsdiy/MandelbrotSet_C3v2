//	フォント
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/03	AVR版をESP32へ移植
//	2025/10	全体改造

#pragma	once

#include <Arduino.h>

class Font
{
private:
	uint8_t	dotX, dotY;	//余白を含まない1文字の大きさ（ドットで定義されている大きさ）
	uint8_t	gapX, gapY;	//余白の大きさ（文字間の隙間：各1ドットを想定）
	uint8_t	fontDataBuf[8];	//Font5x7は元のデータ(90度転倒)では1文字5byte, 正位置に直すと1文字7byte
							//さらに、描画処理を簡単にするため余白Y（1ドット）を含めたバッファサイズとする
	const	uint8_t*	GetGlyph(char cc);

public:
	void	Initialize();
	uint8_t	CharW() const { return dotX + gapX; }
	uint8_t	CharH() const { return dotY + gapY; }
	const	uint8_t*	GetFontData(char cc);
	const	uint8_t*	GetFontDataAsSegmentFormat(char cc);
};
