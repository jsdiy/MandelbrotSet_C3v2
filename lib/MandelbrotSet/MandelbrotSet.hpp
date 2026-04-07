//マンデルブロ集合
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10	初版
//	2026/01	double版に改造
//	2026/04	高速アルゴリズム導入

#pragma once

#include <Arduino.h>
#include <functional>
#include "LcdILI9225.hpp"

//集合領域の設定
namespace	MandelbrotSetConfig
{
	//複素平面の範囲
	//・x,y(=Re,Im)とも閉区間[-2.0, 2.0]で指定する。
	//・画面の縦横比と同じにすると見た目がよくなる。
	constexpr	double
		MinX = -2.0,	MaxX = 1.5,	//実部(Re)の範囲
		MinY = -1.3,	MaxY = 1.3;	//虚部(Im)の範囲

	//中心座標（c=a+bi の a(Re)とb(Im)）
	//・a,bとも閉区間[-2.0, 2.0]で指定する。例: FocusCA = 0.334958, FocusCB = -0.046732
	//・「複素平面の範囲」の中心から開始する場合は0xFFを指定する。
	constexpr	double
		FocusCA = 0xFF,
		FocusCB = 0xFF;

	//拡大レベル（初期値は0）
	//・「中心座標」を指定する場合、拡大レベルも指定したい場合がある。例：Lv:8 C(Re:0.113132,Im:0.644894)
	//・Zoom=4で「複素平面の範囲」がfloat型の場合、レベル10以上(4^10倍に拡大)は実数精度が追従できず、描画が粗くなる。
	//・double版はレベル25以上(4^25倍に拡大)以降で描画が粗くなる。
	constexpr	uint8_t	Level = 0;
}

class	MandelbrotSet
{
public:
	static	constexpr	uint16_t	IterMax = 256;	// 最大反復回数
	static	constexpr	int8_t	CalcStep = 4;	//高速化のため何画素刻みで計算するか(4x4)	※大きすぎると効率が悪い

protected:
	static	constexpr	uint8_t	Zoom = 4;	//1回の操作で何倍に拡大するか

protected:
	int16_t	ScreenW, ScreenH;
	double	minX, minY, maxX, maxY;
	double	centerX, centerY;
	uint8_t	level;
	double	*aBuf = nullptr, *bBuf = nullptr;
	volatile	bool	reqDrawStop;
	uint16_t	Iterate(double a, double b);
	uint16_t	Iterate(int16_t screenX, int16_t screenY);
	uint16_t*	iterBuffer = nullptr;
	size_t	iterBufLength;	//配列の要素数（byte長ではない）
	void	SetComplexPlane(double centerX, double centerY, uint8_t level);
	double	MappingToRe(int16_t screenX);
	double	MappingToIm(int16_t screenY);

public:
	MandelbrotSet() {}
	void	Initialize(int16_t screenWidth, int16_t screenHeight);
	bool	StartCalculation();	//true:計算完了, false:計算中断
	void	StopCalculation() { reqDrawStop = true; }
	void	SetFocus(int16_t screenX, int16_t screenY);
	void	ZoomIn();
	void	ZoomOut();

//PixelHandler
public:
	using	PixelHandler = std::function<void(int16_t, uint16_t*, size_t)>;
	void	SetPixelHandler(PixelHandler func) { FnPixelHandler = func; }

private:
	PixelHandler	FnPixelHandler = nullptr;

//TInfo
public:
	struct	TInfo { uint8_t level; double centerX, centerY, reMin, reMax, imMin, imMax; };
	const	TInfo&	GetInfo() const { return info; }

private:
	TInfo	info;
	void	SetInfo();
};

/*	マンデルブロ集合
式:
	Z[n+1] = Z[n]^2 + c
ただし、
	Z[n] = x[n] + y[n]i
	c = a + bi	※複素平面上の任意の点
	Z[0] = 0 (x = y = 0)
また、
	発散条件: |Z[n]| > 2

描画:
	式を反復計算し、発散するか、一定回数を超えたら反復を終了する。

計算の過程:
Z[n+1] = Z[n]^2 + c
= (x[n] + y[n]i)^2 + (a + bi)
= (x[n]^2 − y[n]^2 + 2x[n]y[n]i) + (a + bi)
= (x[n]^2 − y[n]^2 + a) + (2x[n]y[n] + b)i
== x[n+1] + y[n+1]i	※恒等的に等しい
よって、
x[n+1] = x[n]^2 − y[n]^2 + a,
y[n+1] = 2x[n]y[n] + b

発散の判断:
|Z[n]| > 2
--> Z[n]^2 > 2^2
--> x[n]^2 + y[n]^2 > 4
よって、反復を繰り返す中で↑この式を満たした時点で発散が確定する。
*/
