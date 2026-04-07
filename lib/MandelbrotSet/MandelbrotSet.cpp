//マンデルブロ集合
//『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10	初版
//	2026/01	double版に改造

#include <Arduino.h>
#include "MandelbrotSet.hpp"

//初期化
void	MandelbrotSet::Initialize(int16_t screenWidth, int16_t screenHeight)
{
	using	namespace	MandelbrotSetConfig;
	ScreenW = screenWidth;
	ScreenH = screenHeight;
	
	//反復計算結果格納バッファ
	//iterBufLength = ScreenW;	//配列の要素数
	iterBufLength = ScreenW * CalcStep;	//配列の要素数
	auto iterBufSizeBytes = iterBufLength * sizeof(uint16_t)/*RGB565*/;
	iterBuffer = (uint16_t*)heap_caps_malloc(iterBufSizeBytes, MALLOC_CAP_32BIT);
	Serial.printf("MandelbrotSet:: iterBuffer:0x%X, length=[%d], size=%dbytes\n", iterBuffer, iterBufLength, iterBufSizeBytes);

	//c=a+biの事前計算用バッファ
	aBuf = (double*)heap_caps_malloc(sizeof(double) * ScreenW, MALLOC_CAP_32BIT);
	bBuf = (double*)heap_caps_malloc(sizeof(double) * ScreenH, MALLOC_CAP_32BIT);
	Serial.printf("MandelbrotSet:: aBuf:0x%X, bBuf:0x%X\n", aBuf, bBuf);

	if (FocusCA == 0xFF || FocusCB == 0xFF)
	{
		centerX = (MinX + MaxX) / 2.0;
		centerY = (MinY + MaxY) / 2.0;
	}
	else
	{
		centerX = FocusCA;
		centerY = FocusCB;
	}

	level = Level;
	SetComplexPlane(centerX, centerY, level);
}

//LCDへ描画する
//戻り値	true:完了, false:中断
#if (0)	//1ラインずつ計算
bool	MandelbrotSet::Draw()
{
	auto startMSec = millis();
	reqDrawStop = false;

	for (int16_t y = 0; y < ScreenH; y++)
	{
		double b = bBuf[y];	//c=a+biのb
		for (int16_t x = 0; x < ScreenW; x++)
		{
			//マンデルブロ集合の計算
			double a = aBuf[x];	//c=a+biのa
			uint16_t iterCount = Iterate(a, b);
			iterBuffer[x] = iterCount;
		}

		FnPixelHandler(y, iterBuffer, iterBufLength);
		if (reqDrawStop) { break; }
	}

	Serial.printf("%lu msec\n", millis() - startMSec);
	return !reqDrawStop;
}
#else	//高速版
bool	MandelbrotSet::StartCalculation()
{
	auto startMSec = millis();
	reqDrawStop = false;

	for (int16_t y = 0; y < ScreenH; y += CalcStep)
	{
		for (int16_t x = 0; x < ScreenW; x += CalcStep)
		{
			//4隅を計算する
			auto iterLT = Iterate(x, y);
			auto iterRT = Iterate(x + CalcStep - 1, y);
			auto iterLB = Iterate(x, y + CalcStep - 1);
			auto iterRB = Iterate((int16_t)(x + CalcStep - 1), (int16_t)(y + CalcStep - 1));

			//4隅が同値だったらその値で埋める／そうでなかったら全点計算する
			bool isEqual = (iterLT == iterRT) && (iterLT == iterLB) && (iterLT == iterRB);
			for (int16_t bufPosY = 0, sy = y; sy < y + CalcStep; sy++, bufPosY += ScreenW)
			{
				for (int16_t sx = x; sx < x + CalcStep; sx++)
				{
					iterBuffer[bufPosY + sx] = isEqual ? iterLT : Iterate(sx, sy);
					//毎回判定することになるがスピードの影響は大したことないので気にしない
				}
			}
		}

		FnPixelHandler(y, iterBuffer, iterBufLength);
		if (reqDrawStop) { break; }
	}

	Serial.printf("%lu msec\n", millis() - startMSec);
	return !reqDrawStop;
}
#endif

//マンデルブロ集合を反復計算する
//引数:	画面座標
uint16_t	MandelbrotSet::Iterate(int16_t screenX, int16_t screenY)
{
	//マンデルブロ集合の計算
	double a = aBuf[screenX];	//c=a+biのa
	double b = bBuf[screenY];	//c=a+biのb
	uint16_t iterCount = Iterate(a, b);
	return iterCount;
}

//マンデルブロ集合を反復計算する
//引数:	c=a+biのaとb
uint16_t	MandelbrotSet::Iterate(double a, double b)
{
	// マンデルブロ集合の計算
	double x = 0.0, y = 0.0;	//z[0] = 0
	uint16_t iter;
	for (iter = 0; iter < IterMax; iter++)
	{
		double x2 = x * x;
		double y2 = y * y;
		if (x2 + y2 > 4.0) { break; }	//発散
		y = 2 * x * y + b;	// z[n+1]の虚部	|こちらの式を下の式より先に計算する必要がある。
		x = x2 - y2 + a;	// z[n+1]の実部	|左辺xは新たなx[n+1]を表すが、上の式のxはx[n]であるので。
	}

	return iter;
}

//計算の中心位置を指定する
void	MandelbrotSet::SetFocus(int16_t screenX, int16_t screenY)
{
	centerX = MappingToRe(screenX);
	centerX = std::min(2.0, centerX);
	centerX = std::max(-2.0, centerX);

	centerY = MappingToIm(screenY);
	centerY = std::min(2.0, centerY);
	centerY = std::max(-2.0, centerY);

	SetComplexPlane(centerX, centerY, level);
}

//画面座標を複素平面へマッピングする（c=a+bi の a）
double	MandelbrotSet::MappingToRe(int16_t screenX)
{
	return minX + (maxX - minX) * (double)screenX / (double)(ScreenW - 1);
}

//画面座標を複素平面へマッピングする（c=a+bi の b）
double	MandelbrotSet::MappingToIm(int16_t screenY)
{
	//Y軸とIm軸の向きが逆であることを考慮する
	return maxY - (maxY - minY) * (double)screenY / (double)(ScreenH - 1);
}

//拡大
void	MandelbrotSet::ZoomIn()
{
	level++;
	SetComplexPlane(centerX, centerY, level);
}

//縮小
void	MandelbrotSet::ZoomOut()
{
	if (0 < level)
	{
		level--;
		SetComplexPlane(centerX, centerY, level);
	}
}

//計算対象となる集合領域を決める
//・拡大／縮小は計算誤差を減らすため、現在の階層から次の階層を計算するのではなく、初期値を起点にして計算する。
void	MandelbrotSet::SetComplexPlane(double centerX, double centerY, uint8_t level)
{
	using	namespace	MandelbrotSetConfig;
	double div = std::pow(Zoom, level);
	double initLength, newLength;

	initLength = (MaxX - MinX);
	newLength = initLength / div;
	minX = centerX - newLength / 2.0;
	maxX = centerX + newLength / 2.0;

	initLength = (MaxY - MinY);
	newLength = initLength / div;
	minY = centerY - newLength / 2.0;
	maxY = centerY + newLength / 2.0;

	//集合領域が決まったので、画面座標に対応するc=a+biのa,bを事前に計算しておく
	for (int16_t x = 0; x < ScreenW; x++) { aBuf[x] = MappingToRe(x); }
	for (int16_t y = 0; y < ScreenH; y++) { bBuf[y] = MappingToIm(y); }

	Serial.printf("Lv:%d C(%f,%f) Re(%f,%f) Im(%f,%f)\n", level, centerX, centerY, minX, maxX, minY, maxY);
	SetInfo();
}

//計算情報を格納する
void	MandelbrotSet::SetInfo()
{
	info.level = level;
	info.centerX = centerX;	info.centerY = centerY;
	info.reMin = minX;	info.reMax = maxX;
	info.imMin = minY;	info.imMax = maxY;
}
