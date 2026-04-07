//	グラフィックLCDライブラリ拡張：テキスト描画クラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "GLcdText.hpp"
#include "SpiDma.hpp"

//初期化
void	GLcdText::Initialize()
{
	font.Initialize();
	SetTextScale(1, 1);
}

//文字の色（前景色）を設定する
void	GLcdText::SetTextColor(const Color& color)
{
	foreColor = color;
}

//文字画面の色（背景色）を設定する
void	GLcdText::SetTextBgColor(const Color& color)
{
	bgColor = color;
}

//描画の拡大率を設定する
bool	GLcdText::SetTextScale(uint8_t xW, uint8_t xH)
{
	widthScale = (0 < xW) ? xW : 1;
	heightScale = (0 < xH) ? xH : 1;

	scaledCharW = font.CharW() * widthScale;
	scaledCharH = font.CharH() * heightScale;

	//描画用のバッファを取得する
	size_t reqBufSize = scaledCharW * scaledCharH * Color::Length;	//1文字に必要なバッファサイズ
	bool isOK = (reqBufSize < spiDma.BufferSize());	//DMAバッファのサイズで対応できるか否か
	charImgBuffer = isOK ? spiDma.GetBuffer() : nullptr;	//DMAバッファを割り当てる
	charImgBufSize = isOK ? reqBufSize : 0;	//spiDma.BufferSize()ではなくreqBufSizeとすることに注意
	Serial.printf("Text scale: %dx%d, request buffer size: %d bytes. buffer ptr: 0x%08X\n",
		widthScale, heightScale, reqBufSize, charImgBuffer);

	return isOK;
}

//スケーリングと文字間隔を考慮した1文字の大きさ
void	GLcdText::GetCharSize(int16_t* charWidth, int16_t* charHeight)
{
	*charWidth = scaledCharW;
	*charHeight = scaledCharH;
}

//桁位置（0以上）のx座標
int16_t	GLcdText::XFromCol(uint8_t column) const
{
	return scaledCharW * column;
}

//行位置（0以上）のy座標
int16_t	GLcdText::YFromRow(uint8_t row) const
{
	return scaledCharH * row;
}

//文字を描く
//引数:	開始位置
//戻り値：	次に描く文字のx座標	※文字を描かなかった場合はxは変化しない
int16_t	GLcdText::DrawChar(int16_t x, int16_t y, char cc)
{
	//スケーリングを考慮した1文字が画面に収まるか
	//・縦横とも余白を含めた大きさで判定する。文字画像バッファへは余白込みで描画しているため。
	//・文字が画面の端を超える（文字が欠ける）場合は描かない。
	int16_t screenWidth = Width(), screenHeight = Height();
	if (	(0 <= x) && (x + scaledCharW <= screenWidth)	&&
			(0 <= y) && (y + scaledCharH <= screenHeight)	)
	{
		auto fontDatas = font.GetFontData(cc);
		DrawCharToImageBuffer(fontDatas, charImgBuffer);
		DrawCharImageToDsplay(x, y, scaledCharW, scaledCharH, charImgBuffer, charImgBufSize);
		x += scaledCharW;
	}
	return x;
}

//色データを画素数分、画像バッファへ出力する
size_t	GLcdText::WriteColorToImageBuffer(const Color& color, int16_t repeat, size_t bufIndex, uint8_t* imgBuf)
{
	while (repeat--)
	{
		imgBuf[bufIndex++] = color.Bytes[0];
		imgBuf[bufIndex++] = color.Bytes[1];
	}
	return bufIndex;
}

//スケーリングされた文字を画像バッファに描く（内部処理）
void	GLcdText::DrawCharToImageBuffer(const uint8_t* fontDatas, uint8_t* imgBuf)
{
	//文字データの高さの分、繰り返す
	size_t bufIdx = 0;
	for (uint8_t idxY = 0; idxY < font.CharH(); idxY++)
	{
		uint8_t lineData = fontDatas[idxY];
		size_t startIndex = bufIdx;

		//文字データの幅の分、繰り返す
		for (uint8_t idxX = 0; idxX < font.CharW(); idxX++)
		{
			//画素の色を決める（前景色か背景色か）
			uint8_t pixel = lineData & (0x80 >> idxX);
			Color& color = (pixel != 0) ? foreColor : bgColor;

			//拡大倍数の分、1画素の色データを出力する
			bufIdx = WriteColorToImageBuffer(color, widthScale, bufIdx, imgBuf);
		}

		//いま出力した1行を拡大倍数の分、画像バッファ上で2行目、3行目…へとコピーする
		size_t length = bufIdx - startIndex;	//1行分の描いたデータの長さ
		for (uint8_t lineRepeat = 0; lineRepeat < heightScale - 1; lineRepeat++)
		{
			memcpy(&imgBuf[bufIdx], &imgBuf[startIndex], length);
			bufIdx += length;
		}
	}
}

//画面に文字を描く
void	GLcdText::DrawCharImageToDsplay(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* dmaBuf, size_t bufLength)
{
	BeginSendGRamData(x, y, w, h);
	SendGRamData(dmaBuf, bufLength);
	EndSendGRamData();
}

//文字列を描く
//引数	x,y:	開始位置
//		s:	文字列
//戻り値：	次に描く文字のx座標
int16_t GLcdText::DrawString(int16_t x, int16_t y, const char* s)
{
	int16_t screenWidth = Width();
	while (x + scaledCharW < screenWidth)
	{
		char cc = *(s++);
		if (cc == '\0') { break; }
		x = DrawChar(x, y, cc);
	}
	return x;
}

#if (0)	//Printf()へ置き換え
//WORD値を表示する
//戻り値：	次に描く文字のx座標
int16_t	GLcdText::DrawWord(int16_t x, int16_t y, uint16_t n)
{
	x = DrawByte(x, y, (n >> 8) & 0xFF);
	x = DrawByte(x, y, n & 0xFF);
	return x;
}

//BYTE値を表示する
//戻り値：	次に描く文字のx座標
int16_t	GLcdText::DrawByte(int16_t x, int16_t y, uint8_t n)
{
	char cc;
	cc = ToHexChar((n >> 4) & 0x0F);
	x = DrawChar(x, y, cc);
	cc = ToHexChar(n & 0x0F);
	x = DrawChar(x, y, cc);
	return x;
}

//4bit値を16進数表現の文字に変換する
char	GLcdText::ToHexChar(uint8_t n) const
{
	char cc = (n < 10) ? ('0' + n) : ('A' + (n - 10));
	return cc;
}

//整数を表示する(int32_t: -2,147,483,648～2,147,483,647)
//戻り値：	次に描く文字のx座標
int16_t	GLcdText::DrawInt(int16_t x, int16_t y, int32_t n)
{
	uint32_t m;

	if (n < 0)
	{
		x = DrawChar(x, y, '-');
		m = -n;
	}
	else
	{
		m = n;
	}

	x = DrawUInt(x, y, m);
	return x;
}

//32bit正数を表示する(uint32_t: 0～4,294,967,295)
//戻り値：	次に描く文字のx座標
int16_t	GLcdText::DrawUInt(int16_t x, int16_t y, uint32_t n)
{
	uint8_t	kbuf[10];	//32bit整数は10進数で最大10桁
	int8_t	i = 0;

	do
	{
		kbuf[i++] = n % 10;
		n /= 10;
	}
	while (0 < n);
	
	while (i != 0) { x = DrawChar(x, y, '0' + kbuf[--i]); }
	return x;
}
#endif

#if (0)	//廃止
//ラベル付きDrawWord
int16_t	GLcdText::DrawWord(int16_t x, int16_t y, const char* labelPre, uint16_t n, const char* labelPost)
{
	x = DrawString(x, y, labelPre);
	x = DrawWord(x, y, n);
	if (labelPost != nullptr) { x = DrawString(x, y, labelPost); }
	return x;
}

//ラベル付きDrawByte
int16_t	GLcdText::DrawByte(int16_t x, int16_t y, const char* labelPre, uint8_t n, const char* labelPost)
{
	x = DrawString(x, y, labelPre);
	x = DrawByte(x, y, n);
	if (labelPost != nullptr) { x = DrawString(x, y, labelPost); }
	return x;
}

//ラベル付きDrawInt
int16_t	GLcdText::DrawInt(int16_t x, int16_t y, const char* labelPre, int32_t n, const char* labelPost)
{
	x = DrawString(x, y, labelPre);
	x = DrawInt(x, y, n);
	if (labelPost != nullptr) { x = DrawString(x, y, labelPost); }
	return x;
}

//ラベル付きDrawUInt
int16_t	GLcdText::DrawUInt(int16_t x, int16_t y, const char* labelPre, uint32_t n, const char* labelPost)
{
	x = DrawString(x, y, labelPre);
	x = DrawUInt(x, y, n);
	if (labelPost != nullptr) { x = DrawString(x, y, labelPost); }
	return x;
}
#endif
