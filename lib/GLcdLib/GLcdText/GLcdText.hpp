//	グラフィックLCDライブラリ拡張：テキスト描画クラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10	初版
//	2026/02	ラベル付きDraw関数を追加, PointX/Y()をXFromCol()/YFromRow()へ名称変更

#pragma	once

#include <Arduino.h>
#include "IGLcd.hpp"
#include "Font.hpp"
#include "Color.hpp"

//テキスト描画
class	GLcdText	: public virtual IGLcd
{
private:
	Font	font;
	Color	foreColor = Color(0xFF, 0xFF, 0xFF);	//文字の前景色
	Color	bgColor = Color(0x00, 0x00, 0x00);	//文字の背景色
	uint8_t	widthScale, heightScale;	//縦横n倍
	int16_t	scaledCharW, scaledCharH;	//スケーリングした文字の大きさ（文字の余白を含む）
	uint8_t*	charImgBuffer = nullptr;	//1文字を描画する画像バッファ（プレースホルダ）
	size_t	charImgBufSize;
	size_t	WriteColorToImageBuffer(const Color& color, int16_t repeat, size_t bufIndex, uint8_t* imgBuf);
	void	DrawCharToImageBuffer(const uint8_t* fontDatas, uint8_t* imgBuf);
	void	DrawCharImageToDsplay(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* dmaBuf, size_t bufLength);
	//char	ToHexChar(uint8_t n) const;

	public:
	GLcdText() {}
	void	Initialize();
	void	SetTextColor(const Color& foreColor, const Color& bgColor) { SetTextColor(foreColor); SetTextBgColor(bgColor); }
	void	SetTextColor(const Color& color);	//文字の前景色
	void	SetTextBgColor(const Color& color);	//文字の背景色
	bool	SetTextScale(uint8_t xW, uint8_t xH);	//文字の拡大率
	void	GetCharSize(int16_t* charWidth, int16_t* charHeight);	//拡大率と文字間隔を考慮した1文字の大きさ
	int16_t	XFromCol(uint8_t column) const;	//桁位置(>=0)のx座標
	int16_t	YFromRow(uint8_t row) const;	//行位置(>=0)のy座標
	int16_t	DrawChar(int16_t x, int16_t y, char cc);
	int16_t	DrawString(int16_t x, int16_t y, const char* s);
	int16_t	DrawWord(int16_t x, int16_t y, uint16_t n, const char* prefix = "") { return Printf(x, y, "%s%04X", prefix, n); }
	int16_t	DrawByte(int16_t x, int16_t y, uint8_t n, const char* prefix = "") { return Printf(x, y, "%s%02X", prefix, n); }
	int16_t	DrawInt(int16_t x, int16_t y, int32_t n) { return Printf(x, y, "%d", n); }
	int16_t	DrawUInt(int16_t x, int16_t y, uint32_t n) { return Printf(x, y, "%d", n); }
	//int16_t	DrawWord(int16_t x, int16_t y, const char* labelPre, uint16_t n, const char* labelPost = nullptr);
	//int16_t	DrawByte(int16_t x, int16_t y, const char* labelPre, uint8_t n, const char* labelPost = nullptr);
	//int16_t	DrawInt(int16_t x, int16_t y, const char* labelPre, int32_t n, const char* labelPost = nullptr);
	//int16_t	DrawUInt(int16_t x, int16_t y, const char* labelPre, uint32_t n, const char* labelPost = nullptr);

	template<typename... Args>
	int16_t	Printf(int16_t x, int16_t y, const char* format, Args... args);

protected:
	virtual	~GLcdText() = default;
};

//printf()
//最大100文字までで、'\t' や '\n' など制御コードには非対応
template<typename... Args>
int16_t	GLcdText::Printf(int16_t x, int16_t y, const char* format, Args... args)
{
	char buf[100 + 1];
	size_t charLength = snprintf(buf, 100, format, args...);
	x = DrawString(x, y, buf);
	return x;
}
