//	画像データを取り回すクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/01	旧版を全面改訂

#pragma	once

#include <Arduino.h>

enum	class	EImageFormat	: uint8_t
{
	Custom,			//ユーザー任意（下記にない場合はこれ。モノクロとかBMPファイル丸ごととか）
	RGB888, RGB666,	//3byte/1pix
	RGB565, RGB555,	//2byte/1pix
	RGB444,			//3byte/2pix
	Grayscale256,	//1byte/1pix (8bit/1pix)
	Grayscale16,	//1byte/2pix (4bit/1pix)
};

class	Image
{
protected:
	uint8_t*	buffer;	//プレースホルダ（Imageクラスでnew/mallocしない）
	size_t	bufLength;
	int16_t	width, height;
	EImageFormat	format;
	uint8_t	pixelByte, pixelUnit;
	void	SetPixelParam(EImageFormat fmt);
	size_t	PixelCount(int16_t x, int16_t y) const { return width * y + x; }	//(0,0)から(x,y)までの画素数

public:
	Image() : buffer(nullptr) {}
	Image(EImageFormat format, int16_t width, int16_t height, uint8_t* imgDatas, size_t dataLength)
		: format(format), width(width), height(height), buffer(imgDatas), bufLength(dataLength) { SetPixelParam(format); }
	~Image() { buffer = nullptr; }	//明示的に手放すだけ（Imageクラスでdelete[]/freeしてはいけない）
	void	SetImage(EImageFormat format, int16_t width, int16_t height, uint8_t* imgDatas, size_t dataLength);
	uint8_t*	GetBuffer(int16_t x = 0, int16_t y = 0) const;
	size_t	BufLength(int16_t x = 0, int16_t y = 0) const;
	size_t	DataLengthOf(size_t pixelCount) const;
	int16_t	Width() const { return width; }
	int16_t	Height() const { return height; }
	EImageFormat	Format() const { return format; }
	bool	IsValid() const { return (0 < width) && (0 < height) && (buffer != nullptr) && (0 < bufLength); }
};
