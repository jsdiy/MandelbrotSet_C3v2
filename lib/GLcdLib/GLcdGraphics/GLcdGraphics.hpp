//	図形描画
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/04 - 2025/10	初版, メンテナンス
//	2026/01	DrawRect(bold)を追加

#pragma	once

#include <Arduino.h>
#include "IGLcd.hpp"
#include "Color.hpp"

//図形描画
class	GLcdGraphics	: public virtual IGLcd
{
private:
	void	SwapPoint(int16_t* x1, int16_t* y1, int16_t* x2, int16_t* y2) const;
	void	DrawLineH(int16_t x1, int16_t y1, int16_t x2, const Color& color) const;
	void	DrawLineV(int16_t x1, int16_t y1, int16_t y2, const Color& color) const;
	void	FillBetweenLine(int16_t x1s, int16_t y1s, int16_t x1e, int16_t y1e,
				int16_t x2s, int16_t y2s, int16_t x2e, int16_t y2e, const Color& color) const;
	void	DrawCircleWithFillFlag(int16_t cx, int16_t cy, int16_t r, uint8_t isFill, const Color& color) const;

public:
	void	FillRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color) const;
	void	ClearScreen(const Color& color) const { FillRect(0, 0, Width(), Height(), color); }
	void	DrawPixel(int16_t x, int16_t y, const Color& color) const;
	void	DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const Color& color) const;
	void	DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color) const;
	void	DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t bold, const Color& color) const;
	void	DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const Color& color) const;
	void	FillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const Color& color) const;
	void	DrawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color) const;
	void	FillCircle(int16_t cx, int16_t cy, int16_t r, const Color& color) const;

protected:
	virtual	~GLcdGraphics() = default;
};
