//	色データクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/04 - 2025/08	初版, メンテナンス
//	2026/01	SwapRandB()作成

#pragma	once

#include <Arduino.h>

//色データ
class	Color
{
public:
	static	constexpr	uint8_t	Length = 2;
	uint8_t	Bytes[Length];

	Color(uint8_t red, uint8_t green, uint8_t blue) { SetRGB565(red, green, blue); }
	void	SetRGB565(uint8_t red, uint8_t green, uint8_t blue);
	void	SwapRandB();
};
