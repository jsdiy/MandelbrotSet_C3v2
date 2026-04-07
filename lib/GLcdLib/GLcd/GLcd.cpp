//	SPI-DMA対応グラフィックLCDライブラリ
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "GLcd.hpp"

//初期化
void	GLcd::Initialize(uint8_t spiMode, uint32_t spiClock, gpio_num_t pinSS, gpio_num_t pinDC, int16_t width, int16_t height)
{
	//SPIバスにLCDモジュールを登録する
	bool isOK = glcdSpi.AddDevice(spiMode, spiClock, pinSS, pinDC);
	Serial.printf("SPI mode=%d, clock=%d, pinSS: %d, pinDC: %d, add-dev: %s\n", spiMode, spiClock, pinSS, pinDC, isOK ? "OK" : "NG");
	Serial.printf("SPI DMA buffer:0x%X, bufSize=%d bytes\n", spiDma.GetBuffer(), spiDma.BufferSize());

	//画面サイズを決定する
	screenWidth = LcdWidth = width;
	screenHeight = LcdHeight = height;
	Serial.printf("LCD screen size: %dx%d\n", LcdWidth, LcdHeight);

	//テキスト描画を初期化する
	GLcdText::Initialize();
}

//ハードウェアリセット
void	GLcd::HwReset(gpio_num_t pinRESET) const
{
	if (pinRESET == GPIO_NUM_NC) { return; }
	pinMode(pinRESET, OUTPUT);
	digitalWrite(pinRESET, LOW);	delay(100);
	digitalWrite(pinRESET, HIGH);	delay(300);
}

//回転状態に応じて画面の縦横サイズを入れ替える
void	GLcd::SwapWidthHeight(bool doSwap)
{
	if (doSwap)
	{
		screenWidth = LcdHeight;	screenHeight = LcdWidth;
	}
	else
	{
		screenWidth = LcdWidth;		screenHeight = LcdHeight;
	}
}

//GRAMデータ送信（begin-データ送信-end）
/*	説明
GRAMデータ送信のシーケンスを分割したもの。GLcdXxxから呼び出す想定。
BeginSendGRamData()→SendGRamData()→EndSendGRamData()をセットで呼び出す。
このうちSendGRamData()の呼び出し方がGLcdXxxごとにアレンジされる。bufはDMA対応メモリであること。
*/
void	GLcd::BeginSendGRamData(int16_t x, int16_t y, int16_t w, int16_t h) const
{
	glcdSpi.BeginTransaction();
	SendCommandSetGRamArea(x, y, w, h);
	SendCommandWriteGRam();
}
//
void	GLcd::SendGRamData(const uint8_t* dmaBuf, size_t length) const
{
	glcdSpi.SendData(dmaBuf, length);
}
//
void	GLcd::EndSendGRamData() const
{
	glcdSpi.EndTransaction();
}

//画像を描く（クリッピング非対応）
//引数	x,y,w,h:	画面内の矩形領域
//		dmaBuf:		画像データ	※DMA対応メモリであること
//		dataLength:	データ長(byte)
void	GLcd::DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* dmaBuf, size_t dataLength)
{
	BeginSendGRamData(x, y, w, h);
	SendGRamData(dmaBuf, dataLength);
	EndSendGRamData();
}
