//	SPI-DMA対応グラフィックLCDライブラリ
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/01	初版

#pragma	once

#include <Arduino.h>
#include "Color.hpp"
#include "GLcdSpiDma.hpp"	//SPI-DMA
#include "GLcdText.hpp"		//テキスト描画
#include "GLcdGraphics.hpp"	//図形描画
#include "GLcdImage.hpp"	//高度な画像描画

//SPIピンアサイン
//ESP32のVSPI:		SS = GPIO_NUM_5,  MOSI = GPIO_NUM_23, (MISO = GPIO_NUM_19), SCK = GPIO_NUM_18
//ESP32-C3のSPI2:	SS = GPIO_NUM_10, MOSI = GPIO_NUM_7,  (MISO = GPIO_NUM_2),  SCK = GPIO_NUM_6

//反転/回転表示のパラメータ
//指定パターン:	Normal, RotXXX, FlipXXX, RotXXX|FlipXXX
enum	class	ERotFlip	: uint8_t
{
	Normal	= 0x00,	//LCDの正位置（多くの場合、LCDパネルのフレキシブルケーブルを下側とする向き）
	Rot90	= 0x01,	//LCDの正位置に対して画像を90度回転させる（画面座標系が回転する）
	Rot180	= 0x02,	//LCDの正位置に対して画像を180度回転させる（画面座標系が回転する）
	Rot270	= 0x04,	//LCDの正位置に対して画像を270度(-90度)回転させる（画面座標系が回転する）
	FlipHorizontal	= 0x08,	//画像の正位置に対して水平反転（画面座標系は反転しない）
	FlipVertical	= 0x10	//画像の正位置に対して垂直反転（画面座標系は反転しない）
};

//enumのビット演算子'|'(OR)のオーバーロード
//・キャストせず'A|B'と書ける。
constexpr	ERotFlip	operator |(ERotFlip lhs, ERotFlip rhs)
{
	return static_cast<ERotFlip>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

//グラフィックLCDクラス
/*【注意】IGLcdのダイヤモンド継承問題がGLcdで発生する
・GLcdTextとGLcdImageはIGLcdを継承している。
・GLcdから見るとGLcdTextとGLcdImageのどちらのIGLcdを使えばよいか、コンパイラは区別が付かず（ダイヤモンド継承）、
	エラー: direct base 'IGLcd' inaccessible in 'GLcd' due to ambiguity.　が発生し、
	IGLcdの純粋仮想関数をGLcdで実装できない。
・解決策としては、GLcdTextとGLcdImageがIGLcdを継承する際にvirtualを指定する。　※仮想継承(Virtual Inheritance)
	これにより、GLcd内でIGLcdの実体が1つに統合される。IGLcdの純粋仮想関数をGLcdで実装できる。
*/
class	GLcd	: public GLcdText, public GLcdImage, public GLcdGraphics
{
private:
	int16_t	LcdWidth, LcdHeight;	//LCDモジュールとしての画面サイズ（不変）
	int16_t	screenWidth, screenHeight;	//現在の画面サイズ（90度回転を考慮した論理的な画面サイズ）
	void	BeginSendGRamData(int16_t x, int16_t y, int16_t w, int16_t h) const override;
	void	SendGRamData(const uint8_t* dmaBuf, size_t length) const override;
	void	EndSendGRamData() const override;

protected:
	GLcdSpiDma	glcdSpi;
	void	SwapWidthHeight(bool doSwap);	//RotateFlip()から呼び出す想定の縦横入れ替え関数
	virtual	void	SendCommandSetGRamArea(int16_t x, int16_t y, int16_t w, int16_t h) const = 0;	//描画領域を設定する
	virtual	void	SendCommandWriteGRam() const = 0;	//GRAMに色データを書き込むコマンドを実行する

public:
	/*	基底クラスの同名関数を隠蔽せず、オーバーロードとして利用可能にする
		派生クラス(GLcd)に基底クラス(GLcdImage)と同名の関数(DrawImage)を定義すると、基底クラス側の関数は隠蔽される。
		引数が異なっていてもオーバーロードされない。名前解決が型の一致（引数の違い）より優先されるので。
		解決策: 派生クラスでusing宣言し、基底クラスの関数をこちらのスコープに引き込む。
	*/
    using	GLcdImage::DrawImage;
	void	DrawImage(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* dmaBuf, size_t dataLength);

public:
	int16_t	Width() const override { return screenWidth; }
	int16_t	Height() const override { return screenHeight; }
	void	Initialize(uint8_t spiMode, uint32_t spiClock, gpio_num_t pinSS, gpio_num_t pinDC, int16_t width, int16_t height);
	void	HwReset(gpio_num_t pinRESET) const;
	virtual	void	RotateFlip(ERotFlip param) = 0;	//画面の回転・反転

protected:
	virtual	~GLcd() = default;
};
