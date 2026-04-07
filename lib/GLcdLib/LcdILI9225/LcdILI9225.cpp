//	SPI-DMA対応グラフィックLCDライブラリ：ILI9225
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "LcdILI9225.hpp"

//初期化
void	LcdILI9225::Initialize(gpio_num_t pinSS, gpio_num_t pinDC, uint8_t panelSS, uint8_t panelBGR, uint8_t panelRev, int16_t width, int16_t height)
{
	GLcd::Initialize(SpiMode, SpiClock, pinSS, pinDC, width, height);
	uint16_t bitSS = panelSS, bitBGR = panelBGR, bitREV = panelRev;

	//SPIバス占有
	glcdSpi.BeginTransaction();

	//データシート	13.4. Power Supply Configuration
	//電力系
	delay(20);	//PowerOnReset（1ms以上）+ OscillatorStabilizing time（10ms以上）
	//Power supply operation setting (1)
	SendRegVal(ERegIdx::IdxPOWER1, 0x0F00);	//SAP[3:0]=1111(Fast4[Fastest]):0x0F00, 1000(MediumFast1):0x0800	※'1000'未満にする必要はない
	SendRegVal(ERegIdx::IdxPOWER2, 0x103B);	//APON=1(PON[3:0]=0000),AON=1,VCI1EN=1,VC[3:0]=2.76V
	delay(50);	//Step-up circuit stabilizing time（40ms以上）
	//Power supply operation setting (2)
	//IdxPOWER3,4,5: 初期値のままとする
	
	//画面系
	regVal.R01DRIVER = (bitSS << 8) | 0x001C;	//SM-GS=00, NL[4:0]=11100(528[176xRGB]x220)
	regVal.R03ENTRY = (bitBGR << 12) | 0x0030;	//{I/D[1:0]=11, AM=0}:標準の向き（回転/反転なし）
	SendRegVal(ERegIdx::IdxDRIVER, regVal.R01DRIVER);
	SendRegVal(ERegIdx::IdxENTRY, regVal.R03ENTRY);

	//表示領域系
	/*	前提：標準の向き。表示領域は画面全域。
	・RAMAddress（R20-R21：描画開始位置）の初期値は左上を指しているので、リセット直後は設定不要。
	・WindowAddress（R36-R39：矩形領域）の初期値は画面全域を指しているので、リセット直後は設定不要。
	*/
	
	//画面表示オン
	regVal.R07DISPLAY = (bitREV << 2) | 0x0013;	//{GON=1, D[1:0]=11}:表示オン	※{=0, =00}で表示オフ
	SendRegVal(ERegIdx::IdxDISPLAY, regVal.R07DISPLAY);
	
	//SPIバス解放
	glcdSpi.EndTransaction();
}

//画面を回転／反転させる
/*	データシート	8.2.6. Entry Mode (R03h)
「F」の図の位置と回転／反転の関係
8VHN	(8:180, Vertical, Horizontal, Normal[F])
b79a	(b:90+V, 7:270, 9:90, a:90+H)
*/
void	LcdILI9225::RotateFlip(ERotFlip param)
{
	uint16_t val = 0;

	switch (param)
	{
	//画面縦長グループ
	default:
	case	ERotFlip::Normal:
		val = BitVal::EntryAM_dirH | BitVal::EntryID_HincVinc;
		break;

	case	ERotFlip::FlipHorizontal:
	case	ERotFlip::Rot180 | ERotFlip::FlipVertical:
		val = BitVal::EntryAM_dirH | BitVal::EntryID_HdecVinc;
		break;

	case	ERotFlip::FlipVertical:
	case	ERotFlip::Rot180 | ERotFlip::FlipHorizontal:
		val = BitVal::EntryAM_dirH | BitVal::EntryID_HincVdec;
		break;

	case	ERotFlip::Rot180:
		val = BitVal::EntryAM_dirH | BitVal::EntryID_HdecVdec;
		break;
	
	//画面横長グループ
	case	ERotFlip::Rot90:
		val = BitVal::EntryAM_dirV | BitVal::EntryID_HdecVinc;
		break;

	case	ERotFlip::Rot270:
		val = BitVal::EntryAM_dirV | BitVal::EntryID_HincVdec;
		break;

	case	ERotFlip::Rot90 | ERotFlip::FlipHorizontal:
	case	ERotFlip::Rot270 | ERotFlip::FlipVertical:
		val = BitVal::EntryAM_dirV | BitVal::EntryID_HdecVdec;
		break;

	case	ERotFlip::Rot90 | ERotFlip::FlipVertical:
	case	ERotFlip::Rot270 | ERotFlip::FlipHorizontal:
		val = BitVal::EntryAM_dirV | BitVal::EntryID_HincVinc;
		break;
	}

	regVal.R03ENTRY &= ~BitVal::EntryAMID_BitMask;
	regVal.R03ENTRY |= val;
	glcdSpi.BeginTransaction();
	SendRegVal(ERegIdx::IdxENTRY, regVal.R03ENTRY);
	glcdSpi.EndTransaction();

	//回転状態に応じて画面の縦横サイズを入れ替える
	SwapWidthHeight(val & BitVal::EntryAM_dirV);
}

//データ書込み先のGRAM領域を設定する
//引数:	画面座標系の矩形領域。画面内に収まっていること。
void	LcdILI9225::SendCommandSetGRamArea(int16_t x, int16_t y, int16_t w, int16_t h) const
{
	//回転・反転に応じた座標変換	※S:Source:LCDモジュール正位置での水平方向, G:Gate:垂直方向
	int16_t windowSLeft, windowSRight, windowGTop, windowGBottom;	//描画対象の矩形領域
	int16_t writeSX, writeGY;	//矩形領域のどの四隅が描画開始点か

	//回転を考慮した（＝現在の状態の）画面の縦横サイズ
	const int16_t ScreenWidth = this->Width(), ScreenHeight = this->Height();

	uint16_t val = regVal.R03ENTRY & BitVal::EntryAMID_BitMask;
	switch (val)
	{
	//画面縦長グループ
	default:
	case BitVal::EntryAM_dirH | BitVal::EntryID_HincVinc:	//Normal (Rot0|FlipNone)
		windowSLeft = x;	windowSRight = x + w - 1;	windowGTop = y;	windowGBottom = y + h - 1;
		writeSX = windowSLeft;	writeGY = windowGTop;
		break;
	case BitVal::EntryAM_dirH | BitVal::EntryID_HdecVinc:	//Rot0|FlipHorizontal, Rot180|FlipVertical
		windowSLeft = (ScreenWidth - 1) - (x + w - 1);	windowSRight = (ScreenWidth - 1) - x;	windowGTop = y;	windowGBottom = y + h - 1;
		writeSX = windowSRight;	writeGY = windowGTop;
		break;
	case BitVal::EntryAM_dirH | BitVal::EntryID_HincVdec:	//Rot0|FlipVertical, Rot180|FlipHorizontal
		windowSLeft = x;	windowSRight = x + w - 1;	windowGTop = (ScreenHeight - 1) - (y + h - 1);	windowGBottom = (ScreenHeight - 1) - y;
		writeSX = windowSLeft;	writeGY = windowGBottom;
		break;
	case BitVal::EntryAM_dirH | BitVal::EntryID_HdecVdec:	//Rot180
		windowSLeft = (ScreenWidth - 1) - (x + w - 1);	windowSRight = (ScreenWidth - 1) - x;	windowGTop = (ScreenHeight - 1) - (y + h - 1);	windowGBottom = (ScreenHeight - 1) - y;
		writeSX = windowSRight;	writeGY = windowGBottom;
		break;

	//画面横長グループ
	case BitVal::EntryAM_dirV | BitVal::EntryID_HdecVinc:	//Rot90
		windowSLeft = (ScreenHeight - 1) - (y + h - 1);	windowSRight = (ScreenHeight - 1) - y;	windowGTop = x;	windowGBottom = x + w - 1;
		writeSX = windowSRight;	writeGY = windowGTop;
		break;
	case BitVal::EntryAM_dirV | BitVal::EntryID_HincVdec:	//Rot270
		windowSLeft = y;	windowSRight = y + h - 1;	windowGTop = (ScreenWidth - 1) - (x + w - 1);	windowGBottom = (ScreenWidth - 1) - x;
		writeSX = windowSLeft;	writeGY = windowGBottom;
		break;
	case BitVal::EntryAM_dirV | BitVal::EntryID_HdecVdec:	//Rot90|FlipHorizontal, Rot270|FlipVertical
		windowSLeft = (ScreenHeight - 1) - (y + h - 1);	windowSRight = (ScreenHeight - 1) - y;	windowGTop = (ScreenWidth - 1) - (x + w - 1);	windowGBottom = (ScreenWidth - 1) - x;
		writeSX = windowSRight;	writeGY = windowGBottom;
		break;
	case BitVal::EntryAM_dirV | BitVal::EntryID_HincVinc:	//Rot90|FlipVertical, Rot270|FlipHorizontal
		windowSLeft = y;	windowSRight = y + h - 1;	windowGTop = x;	windowGBottom = x + w - 1;
		writeSX = windowSLeft;	writeGY = windowGTop;
		break;
	}

	//描画領域(Window)をセットする
	SendRegVal(ERegIdx::IdxWNDHST, windowSLeft);
	SendRegVal(ERegIdx::IdxWNDHED, windowSRight);
	SendRegVal(ERegIdx::IdxWNDVST, windowGTop);
	SendRegVal(ERegIdx::IdxWNDVED, windowGBottom);
	//Serial.printf("HStart=%04X HEnd=%04X VStart=%04X VEnd=%04X\n", windowSLeft, windowSRight, windowGTop, windowGBottom);

	//書き込み開始アドレスをセットする
	//GRAM Address 書式: 0xGGSS (SourceがLo, GateがHi)
	SendRegVal(ERegIdx::IdxRAMADL, writeSX);
	SendRegVal(ERegIdx::IdxRAMADH, writeGY);
	//Serial.printf("GRAMAddr: Horizon(LoByte)=%04X Vertical(HiByte)=%04X\n", writeSX, writeGY);
}
