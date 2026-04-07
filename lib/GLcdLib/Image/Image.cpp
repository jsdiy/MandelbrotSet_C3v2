//	画像データの取り回し
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Image.hpp"

//画像情報をセットする
//引数	imgDatas: 画素データ（DMA対応メモリに限らない）
void	Image::SetImage(EImageFormat format, int16_t width, int16_t height, uint8_t* imgDatas, size_t dataLength)
{
	this->format = format;
	this->width = width;
	this->height = height;
	buffer = imgDatas;
	bufLength = dataLength;
	SetPixelParam(format);
}

//画像フォーマットに対応した画素情報をセットする
void	Image::SetPixelParam(EImageFormat fmt)
{
	switch (fmt)
	{
	default:
	case EImageFormat::Custom:
		pixelByte = 1;	pixelUnit = 1;	break;

	case EImageFormat::RGB888:
	case EImageFormat::RGB666:
		pixelByte = 3;	pixelUnit = 1;	break;

	case EImageFormat::RGB565:
	case EImageFormat::RGB555:
		pixelByte = 2;	pixelUnit = 1;	break;

	case EImageFormat::RGB444:
		pixelByte = 3;	pixelUnit = 2;	break;

	case EImageFormat::Grayscale256:
		pixelByte = 1;	pixelUnit = 1;	break;

	case EImageFormat::Grayscale16:
		pixelByte = 1;	pixelUnit = 2;	break;
	}
}

//画像内の指定位置以降の画像データを取得する
uint8_t*	Image::GetBuffer(int16_t x, int16_t y) const
{
	if (x == 0 && y == 0) { return buffer; }	//idxを計算する必要がないので直ちに返す
	if (x < 0 || width <= x || y < 0 || height <= y)	//不正な引数の場合
	{
		Serial.printf("Image::GetBuffer(%d,%d) out of range.\n", x, y);
		return buffer;
	}

	size_t pixelCount = PixelCount(x, y);
	size_t idx = DataLengthOf(pixelCount);
	return &buffer[idx];
}

//画像内の指定位置以降の画像データ長(byte)を取得する
size_t	Image::BufLength(int16_t x, int16_t y) const
{
	if (x == 0 && y == 0) { return bufLength; }	//lengthXYを計算する必要がないので直ちに返す

	size_t pixelCount = PixelCount(x, y);
	size_t dataLengthXY = DataLengthOf(pixelCount);
	return bufLength - dataLengthXY;
}

//画素数分の画像データ長(byte)を取得する
size_t	Image::DataLengthOf(size_t pixelCount) const
{
	pixelCount = (pixelCount / pixelUnit) * pixelUnit;	//画素数はpixelUnitの倍数で指定する必要がある
	return pixelCount * pixelByte / pixelUnit;
}
