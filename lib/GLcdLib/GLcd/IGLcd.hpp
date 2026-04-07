//	SPI-DMA対応グラフィックLCDライブラリ：描画機能のインターフェース
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/02	初版

#pragma	once

#include <Arduino.h>

//論理LCDクラス(GLcd)を拡張するクラス(GLcdXxx)は必要に応じてこのインターフェースを継承すること
//・GLcdで起きうるダイヤモンド継承の対策として仮想継承とすること。　class GLcdXxx : public virtual IGLcd
//・実装はGLcdとする。　※そもそもIGLcdはGLcdクラスの関数をGLcdXxxクラスで使いたくて用意したものなので
class	IGLcd
{
protected:
	virtual	int16_t	Width() const = 0;
	virtual	int16_t	Height() const = 0;
	virtual	void	BeginSendGRamData(int16_t x, int16_t y, int16_t w, int16_t h) const = 0;
	virtual	void	SendGRamData(const uint8_t* buf, size_t length) const = 0;
	virtual	void	EndSendGRamData() const = 0;

public:
	virtual	~IGLcd() = default;
};
