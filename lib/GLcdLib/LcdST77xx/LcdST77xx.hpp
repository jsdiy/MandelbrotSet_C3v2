//	SPI-DMA対応グラフィックLCDライブラリ：ST77xx
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2020/05 - 2025/08	初版（独立版）, メンテナンス
//	2026/01	GLcdクラス共通化

#pragma	once

#include <Arduino.h>
#include "GLcd.hpp"

//グラフィックLCDクラス
class	LcdST77xx	: public GLcd
{
private:	//SPI設定
	static	constexpr	uint8_t		SpiMode = 0;	//SPIモード(0-3)
	static	constexpr	uint32_t	SpiClock = 16UL * 1000000;	//SPIクロック(Hz)	※80MHzの分周値が最適
	//ST7735の4線モードは SCLK_min = 66ns -> SPI_busclock = 15.2MHz
	//ST7789の4線モードは SCL(TSCYCW: Serial clock cycle (Write)) = 66ns [write command & data ram]

private:	//レジスタ関連
	//コマンド	※利用頻度が高そうなもの
	enum	class	Command	: uint8_t
	{
		SWRESET	= 0x01,	//Software reset
		SLPIN	= 0x10,	//Sleep in & booster off	(Sleep-outからSleep-inした場合、120ms待つ必要がある)
		SLPOUT	= 0x11,	//Sleep out & booster on	(Sleep-inからSleep-outした場合、120ms待つ必要がある)
		INVON	= 0x21,	//Display Inversion On	（色反転。RGB_00:00:00が白, FF:FF:FFが黒）
		DISPOFF	= 0x28,	//Display off
		DISPON	= 0x29,	//Display on
		CASET	= 0x2A,	//Column address set
		RASET	= 0x2B,	//Row address set
		RAMWR	= 0x2C,	//Memory write
		MADCTL	= 0x36,	//Memory data access control
		COLMOD	= 0x3A,	//Interface pixel format
	};

	//レジスタのビット値
	enum	BitVal : uint8_t
	{
		//Memory data access control
		//・回転角=MY|MX|MV: 0度=0|0|0, 90度=0|1|1, 180度=1|1|0, 270度=1|0|1
		//・ML,MH: 画像がスクロールしている場合、リフレッシュ方向を設定するとチラつきが軽減される可能性がある。
		MadctlMY_FlipV	= (1 << 7),	//MCUからメモリへの書き込み方向…	0:順方向, 1:逆方向（垂直反転）
		MadctlMX_FlipH	= (1 << 6),	//MCUからメモリへの書き込み方向…	0:順方向, 1:逆方向（水平反転）
		MadctlMV_Rot90	= (1 << 5),	//MCUからメモリへの書き込み方向…	0:水平方向優先, 1:垂直方向優先（90度回転）
		MadctlML	= (1 << 4),	//LCDパネルのリフレッシュ方向…	0:Top行→Bottom行方向, 1:Bottom行→Top行方向
		MadctlRGB	= (1 << 3),	//メモリ上のRGBデータとLCDパネルのRGB画素の並び順の対応…	0:RGB, 1:BGR
		MadctlMH	= (1 << 2),	//LCDパネルのリフレッシュ方向…	0:Left列→Right列方向, 1:Right列→Left列方向
		Madctl_BitMask	= MadctlMY_FlipV | MadctlMX_FlipH | MadctlMV_Rot90,	//ビットが全て'1'
	};

	//レジスタ値	※初期化以降で書き換えが発生するレジスタの値を保持しておく目的
	uint8_t	regValMADCTL;

private:
	void	SendCommandSetGRamArea(int16_t x, int16_t y, int16_t w, int16_t h) const override;
	void	SendCommandWriteGRam() const override { SendCommand(Command::RAMWR); }
	void	SendCommand(Command cmd) const { glcdSpi.SendCommand((uint8_t)cmd); }
	void	SendCommandValue(Command cmd, uint8_t val) const { SendCommand(cmd); glcdSpi.SendData(val); }

public:
	void	Initialize(gpio_num_t pinSS, gpio_num_t pinDC, uint16_t controller, int16_t width, int16_t height, uint8_t panelBGR, uint8_t panelInvOn);
	void	RotateFlip(ERotFlip param) override;
};

/*	ST7735のコマンド一覧
//System Function Command List and Description
static	const	uint8_t
	NOP		= 0x00,	//No Operation
	SWRESET	= 0x01,	//Software reset
	SLPIN	= 0x10,	//Sleep in & booster off	(Sleep-outからSleep-inした場合、120ms待つ必要がある)
	SLPOUT	= 0x11,	//Sleep out & booster on	(Sleep-inからSleep-outした場合、120ms待つ必要がある)
	PTLON	= 0x12,	//Partial mode on
	NORON	= 0x13,	//Partial off (Normal mode on)
	INVOFF	= 0x20,	//Display inversion off
	INVON	= 0x21,	//Display inversion on
	GAMSET	= 0x26,	//Gamma curve select
	DISPOFF	= 0x28,	//Display off
	DISPON	= 0x29,	//Display on
	CASET	= 0x2A,	//Column address set
	RASET	= 0x2B,	//Row address set
	RAMWR	= 0x2C,	//Memory write
	PTLAR	= 0x30,	//Partial start/end address set
	TEOFF	= 0x34,	//Tearing effect line off
	TEON	= 0x35,	//Tearing effect mode set & on
	MADCTL	= 0x36,	//Memory data access control
	IDMOFF	= 0x38,	//Idle mode off
	IDMON	= 0x39,	//Idle mode on
	COLMOD	= 0x3A;	//Interface pixel format

//Panel Function Command List and Description
static	const uint8_t
	FRMCTR1	= 0xB1,	//In normal mode (Full colors)
	FRMCTR2	= 0xB2,	//In Idle mode (8-colors)
	FRMCTR3	= 0xB3,	//In partial mode + Full colors
	INVCTR	= 0xB4,	//Display inversion control
	DISSET5	= 0xB6,	//Display function setting
	PWCTR1	= 0xC0,	//Power control setting
	PWCTR2	= 0xC1,	//Power control setting
	PWCTR3	= 0xC2,	//Power control setting / In normal mode (Full colors)
	PWCTR4	= 0xC3,	//Power control setting / In Idle mode (8-colors)
	PWCTR5	= 0xC4,	//Power control setting / In partial mode + Full colors
	VMCTR1	= 0xC5,	//VCOM control 1
	VMOFCTR	= 0xC7,	//Set VCOM offset control
	WRID2	= 0xD1,	//Set LCM version code
	WRID3	= 0xD2,	//Customer Project code
	NVCTR1	= 0xD9,	//EEPROM control status
	NVCTR2	= 0xDE,	//EEPROM Read Command
	NVCTR3	= 0xDF,	//EEPROM Write Command
	GAMCTRP1	= 0xE0,	//Set Gamma adjustment ('+' polarity)
	GAMCTRN1	= 0xE1,	//Set Gamma adjustment ('-' polarity)
	EXTCTRL	= 0xF0,	//Extension Command Control
	PWCTR6	= 0xFC,	//Power control setting / In partial mode + Idle
	VCOM4L	= 0xFF;	//Vcom 4 Level control
*/
