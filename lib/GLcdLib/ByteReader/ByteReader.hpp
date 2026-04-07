//	byte配列からバッファへ順次読み込むクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/02	初版

#pragma	once

#include <Arduino.h>
#include <functional>

//byte配列を規則的に読み出す
class	ByteReader
{
private:
	struct	Common
	{
		uint8_t*	buf;
		size_t	bufSize;
		const	uint8_t*	datas;
		virtual	size_t	Next() = 0;
	};

	struct Linear : Common
	{
		int16_t	idx;
		size_t	srcLength;
		size_t	Next() override;
	};
	
	struct Strided : Common
	{
		size_t	stepLength, copyLength;
		int16_t	repeatMax, repeatCount;
		size_t	Next() override;
	};

private:
	Linear	linear;
	Strided	strided;

public:
	void	SetLinear(uint8_t* buffer, size_t bufSize, const uint8_t* datas, size_t length);
	void	SetStrided(uint8_t* buffer, size_t bufSize, const uint8_t* datas, size_t stepLength, size_t copyLength, int16_t repeat);
	std::function<size_t()> Next;
};

/*	使い方
void	DrawImage(uint8_t* imgData, size_t imgDataLength)
{
	size_t	bufSize = 4*1024;
	uint8_t*	readBuf = GetBuffer(bufSize);
	ByteReader	br;
	br.SetLinear(readBuf, bufSize, imgData, imgDataLength);
	while (size_t readLength = br.Next()) { WriteGRam(readBuf, readLength); }

	//画像データの指定位置（画像内の矩形領域の左上座標）から、
	//10KBの区間（画像の幅）ごとに、2KB（矩形領域の幅）読むことを、5回（矩形領域の高さ）繰り返す
	br.SetStrided(readBuf, bufSize, &imgData[offset], 10*1024, 2*1024, 5);
	while (size_t readLength = br.Next()) { WriteGRam(readBuf, readLength); }
}
*/
