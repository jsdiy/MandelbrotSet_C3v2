//	SPI-DMA対応グラフィックLCDライブラリ：ILI9225
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/01	初版（独立版）
//	2026/01	GLcdクラス共通化

#pragma	once

#include <Arduino.h>
#include "GLcd.hpp"

//グラフィックLCDクラス
class	LcdILI9225	: public GLcd
{
private:	//SPI設定
	static	constexpr	uint8_t		SpiMode = 0;	//SPIモード(0-3)
	static	constexpr	uint32_t	SpiClock = 40UL * 1000000;	//SPIクロック(Hz)	※80MHzの分周値が望ましい
	/*	データシート	14.6.3. Serial Data Transfer Interface Timing Characteristics
	Serial clock cycle time
	Write ( received )  : tSCYC Min. = 20ns	-> SPI_busclock Max. = 50MHz
	Read ( transmitted ): tSCYC Min. = 40ns	-> SPI_busclock Max. = 25MHz
	*/

private:	//レジスタ関連
	//レジスタ番号	※利用頻度が高そうなもの
	//データシート	8.2. Instruction Descriptions
	enum	class	ERegIdx	: uint8_t
	{
		IdxDRIVER	= 0x01,	//Driver Output Control
		IdxENTRY	= 0x03,	//Entry Mode
		IdxDISPLAY	= 0x07,	//Display Control 1
		IdxPOWER1	= 0x10,	//Power Control 1
		IdxPOWER2	= 0x11,	//Power Control 2
		IdxPOWER3	= 0x12,	//Power Control 3
		IdxPOWER4	= 0x13,	//Power Control 4
		IdxPOWER5	= 0x14,	//Power Control 5
		IdxRAMADL	= 0x20,	//RAM Address Set 1 (Lo-byte)	※下位バイトは短辺方向（ソース）の描画開始位置
		IdxRAMADH	= 0x21,	//RAM Address Set 2	(Hi-byte)	※上位バイトは長辺方向（ゲート）の描画開始位置
		IdxWRGRAM	= 0x22,	//Write Data to GRAM
		IdxSWRESET	= 0x28,	//Software Reset
		IdxWNDHED	= 0x36,	//Horizontal Window Address -1（終了位置）	※水平方向＝短辺方向（ソース）
		IdxWNDHST	= 0x37,	//Horizontal Window Address -2（開始位置）
		IdxWNDVED	= 0x38,	//Vertical Window Address -1（終了位置）	※垂直方向＝長辺方向（ゲート）
		IdxWNDVST	= 0x39,	//Vertical Window Address -2（開始位置）
	};

	//レジスタのビット値
	enum	BitVal : uint16_t
	{
		//回転・反転
		EntryAM_dirH	= (0 << 3),	//0:address update horizontal writing direction, 1:vertical
		EntryAM_dirV	= (1 << 3),
		EntryID_HdecVdec	= (0b00 << 4),	//00,01,10,11	※I/D: Increment/Decrement
		EntryID_HincVdec	= (0b01 << 4),
		EntryID_HdecVinc	= (0b10 << 4),
		EntryID_HincVinc	= (0b11 << 4),
		EntryAMID_BitMask	= EntryAM_dirV | EntryID_HincVinc,	//AMとI/Dのビットが全て'1'
	};

	//レジスタ値	※初期化以降で書き換えが発生するレジスタの値を保持しておく目的
	struct	RegisterValue
	{
		uint16_t
			R01DRIVER,
			R03ENTRY,
			R07DISPLAY;
	};

private:
	static	constexpr	int16_t	LcdWidth = 176;
	static	constexpr	int16_t	LcdHeight = 220;
	RegisterValue	regVal;
	void	SendCommandSetGRamArea(int16_t x, int16_t y, int16_t w, int16_t h) const override;
	void	SendCommandWriteGRam() const override { glcdSpi.SendCommand((uint8_t)ERegIdx::IdxWRGRAM); }
	void	SendRegVal(ERegIdx reg, uint16_t val) const { glcdSpi.SendCommand((uint8_t)reg); glcdSpi.SendData(val); }

public:
	void	Initialize(gpio_num_t pinSS, gpio_num_t pinDC, uint8_t panelSS, uint8_t panelBGR, uint8_t panelRev, int16_t width = LcdWidth, int16_t height = LcdHeight);
	void	RotateFlip(ERotFlip param) override;
};

/*	8.2.18. RAM Address Set (R20h, R21h)
GRAM Address Range
ADDR[15:0]		Gram setting
0000h to 00AFh	Bitmap data for G1
0100h to 01AFh	Bitmap data for G2
0200h to 02AFh	Bitmap data for G3
0300h to 03AFh	Bitmap data for G4
:				:
D800h to D8AFh	Bitmap data for G217
D900h to D9AFh	Bitmap data for G218
DA00h to DAAFh	Bitmap data for G219
DB00h to DBAFh	Bitmap data for G220

※Hi: 00xxh～DBxxh --> 0～219 (220line)
※Lo: xx00h～xxAFh --> 0～175 (176px)
*/
/*	ILI9225の命令一覧
No.	Registers Name
00h	Driver Code Read	※読み込み命令
01h	Driver Output Control
02h	LCD AC Driving Control
03h	Entry Mode
07h	Display Control 1
08h	Blank Period Control 1	(Display Control 2)
0Bh	Frame Cycle Control
0Ch	Interface Control
0Fh	Oscillation Control
10h	Power Control 1
11h	Power Control 2
12h	Power Control 3
13h	Power Control 4
14h	Power Control 5
15h	VCI Recycling
20h	RAM Address Set 1
21h	RAM Address Set 2
22h	Write Data to GRAM
22h	Read Data from GRAM	※読み込み命令（Writeと同じNo.だが、StartByteのR/Wビットで区別する）
28h	Software Reset
30h	Gate Scan Control
31h	Vertical Scroll Control 1
32h	Vertical Scroll Control 2
33h	Vertical Scroll Control 3
34h	Partial Driving Position -1
35h	Partial Driving Position -2
36h	Horizontal Window Address -1
37h	Horizontal Window Address -2
38h	Vertical Window Address -1
39h	Vertical Window Address -2
50h	Gamma Control 1
51h	Gamma Control 2
52h	Gamma Control 3
53h	Gamma Control 4
54h	Gamma Control 5
55h	Gamma Control 6
56h	Gamma Control 7
57h	Gamma Control 8
58h	Gamma Control 9
59h	Gamma Control 10
*/
/*	表示パネル特性：SSビット、BGRビット、REVビットの説明
SSビット - R01 Driver Output Controlレジスタ
	0:水平方向において発光素子がコントローラーの端子と同順で結線されているパネル, 1:逆順のパネル
BGRビット - R03 Entry Modeレジスタ
	0:発光素子の並びがR-G-Bのパネル, 1:B-G-Rのパネル
REVビット - R07 Display Control 1レジスタ
	0:色データのとおりに発色するパネル, 1:白の補色で発色するパネル（白データが黒色、青が黄色、赤が水色など）

■SSビット
SS=0/1は、パネルの水平方向の端子順に合うよう、コントローラーのソース端子のドライブ順を設定する。
	SS=0: S1 → S528		パネルの端子順がコントローラーのソース端子順と同じ場合は0
	SS=1: S528 → S1		パネルの端子順がコントローラーのソース端子順と逆の場合は1
・SS=0で表示される画像に対して、SS=1はRGBが逆順(BGR)の発色、かつ、左右反転で表示される。
・SSビットでは、色の正しさは設定できない。

■BGRビット
BGR=0/1は、パネルの発光素子の並び順(RGB/BGR)に合うよう、コントローラーのソース端子のドライブ順を設定する。
イメージとしては次のようになる。　※数字はソース端子を表し、[]はパネル上の1画素を表す。
	BGR=0: [1,2,3][4,5,6]...[523,524,525][526,527,528]	RGBパネルの場合は0
	BGR=1: [3,2,1][6,5,4]...[525,524,523][528,527,526]	BGRパネルの場合は1
・BGR=0に対してBGR=1はR/B成分が入れ替わった色で表示される。例：赤(#F00)←→青(#00F)
・BGRビットでは画像の左右は反転しない。また、白黒が反転するわけではない。

■REVビット
REV=0/1は、パネルの色濃度の解釈に合うよう、色データのグレースケールを反対にする。
	REV=0: グレースケールはそのまま		色データをそのまま出力する場合は0
	REV=1: グレースケールを反対にする	色データを0xFFFFの補色で出力する場合は1
・画像を表示したときに、白(#FFF)←→黒(#000)、赤(#F00)←→水色(#0FF)、緑(#0F0)←→紫(#F0F)、青(#00F)←→黄(#FF0)
　のように色化けしている場合はREV=1を設定する。

■SSビット、BGRビット、REVビットの設定手順
準備
- R01_DriverOutputControlレジスタに、SS=0 を設定する。
- R03_EntryModeレジスタに、BGR=0, {I/D[1:0]=11, AM=0}:標準の向き を設定する。
- R07_DisplayControl1レジスタに、REV=0 を設定する。
- LCDモジュールを正位置に置く。多くの場合、LCDパネルのフレキシブルケーブルを下側とする向き。
手順
(1) (0,0)-(100,100)の領域を青(0x001F)で塗る。(2)へ。
(2) 色に関係なく、四角形が左上に表示されていたら(3)へ。
	右上に表示されていたらSS=1とし、改めて(1)を実行。
	※左下か右下に表示された場合、LCDモジュールを上下逆さまに置いていることになる。
(3) 四角形が青だったら設定完了。
	赤だったらBGR=1とし、改めて(1)から実行。
	黄だったらREV=1とし、改めて(1)から実行。
	水色だったらBGR=1,REV=1とし、改めて(1)から実行。
*/
