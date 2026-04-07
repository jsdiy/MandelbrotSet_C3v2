//	SPI-DMA対応グラフィックLCDライブラリ：ST77xx
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "LcdST77xx.hpp"

//初期化
void	LcdST77xx::Initialize(gpio_num_t pinSS, gpio_num_t pinDC, uint16_t controller, int16_t width, int16_t height, uint8_t panelBGR, uint8_t panelInvOn)
{
	GLcd::Initialize(SpiMode, SpiClock, pinSS, pinDC, width, height);

	//SPIバス占有
	glcdSpi.BeginTransaction();

	//LCDをリセットする
	SendCommand(Command::SWRESET);	delay(200);	//120ms以上

	//スリープ解除する（リセット直後はスリープ状態にあるので）
	SendCommand(Command::SLPOUT);	delay(200);	//120ms以上

	//カラーモード（色深度）を指定する
	if (controller == 7735) { SendCommandValue(Command::COLMOD, 0x05); }	//RGB:565
	if (controller == 7789) { SendCommandValue(Command::COLMOD, 0x55); }	//RGB:565

	//色反転（これが必要かはLCDモジュールによる）
	if (panelInvOn) { SendCommand(Command::INVON); }

	//画面表示オン
	SendCommand(Command::DISPON);

	//SPIバス解放
	glcdSpi.EndTransaction();
}

//画面を回転／反転させる
void	LcdST77xx::RotateFlip(ERotFlip param)
{
	uint8_t val = 0;

	switch (param)
	{
	//画面縦長グループ
	default:
	case	ERotFlip::Normal:
		val = 0;
		break;

	case	ERotFlip::FlipHorizontal:
	case	ERotFlip::Rot180 | ERotFlip::FlipVertical:
		val = BitVal::MadctlMX_FlipH;
		break;

	case	ERotFlip::FlipVertical:
	case	ERotFlip::Rot180 | ERotFlip::FlipHorizontal:
		val = BitVal::MadctlMY_FlipV;
		break;

	case	ERotFlip::Rot180:
		val = BitVal::MadctlMX_FlipH | BitVal::MadctlMY_FlipV;
		break;
	
	//画面横長グループ
	case	ERotFlip::Rot90:
		val = BitVal::MadctlMV_Rot90;
		break;

	case	ERotFlip::Rot270:
		val = BitVal::MadctlMV_Rot90 | BitVal::MadctlMX_FlipH | BitVal::MadctlMY_FlipV;
		break;

	case	ERotFlip::Rot90 | ERotFlip::FlipHorizontal:
	case	ERotFlip::Rot270 | ERotFlip::FlipVertical:
		val = BitVal::MadctlMV_Rot90 | BitVal::MadctlMX_FlipH;
		break;

	case	ERotFlip::Rot90 | ERotFlip::FlipVertical:
	case	ERotFlip::Rot270 | ERotFlip::FlipHorizontal:
		val = BitVal::MadctlMV_Rot90 | BitVal::MadctlMY_FlipV;
		break;
	}

	regValMADCTL &= ~BitVal::Madctl_BitMask;
	regValMADCTL |= val;
	glcdSpi.BeginTransaction();
	SendCommandValue(Command::MADCTL, val);
	glcdSpi.EndTransaction();

	//回転状態に応じて画面の縦横サイズを入れ替える
	SwapWidthHeight(val & (uint8_t)ERotFlip::Rot90);
}

//データ書込み先のGRAM領域を設定する
//引数:	データ書込み先のGRAM領域。画面内に収まっていること。
void	LcdST77xx::SendCommandSetGRamArea(int16_t x, int16_t y, int16_t w, int16_t h) const 
{
	SendCommand(Command::CASET);
	glcdSpi.SendData((uint16_t)x, (uint16_t)(x + w - 1));	//startX, endX
	SendCommand(Command::RASET);
	glcdSpi.SendData((uint16_t)y, (uint16_t)(y + h - 1));	//startY, endY
}
