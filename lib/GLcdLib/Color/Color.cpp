//	色データクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Color.hpp"

//色をセットする
void	Color::SetRGB565(uint8_t red, uint8_t green, uint8_t blue)
{
	red		= red   * 0x1F / 0xFF;
	green	= green * 0x3F / 0xFF;
	blue	= blue  * 0x1F / 0xFF;
	
	//[R5:G3][G3:B5]
	Bytes[0] = (red   << 3) | (green >> 3);
	Bytes[1] = (green << 5) | blue;
}

//RGB565のRとBを入れ替える
void	Color::SwapRandB()
{
	uint16_t c = ((uint16_t)Bytes[0] << 8) | Bytes[1];
	c = (c & 0x07E0) | (c >> 11) | (c << 11);
	Bytes[0] = highByte(c);
	Bytes[1] = lowByte(c);
}
