//	図形描画
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "GLcdGraphics.hpp"
#include "SpiDma.hpp"

//単色で矩形領域を塗りつぶす
//引数	x,y,w,h:	画面内の矩形領域
//		color:		RGB565の色データ（LCDのカラーモードがRGB565に設定されていることが前提）
void	GLcdGraphics::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color) const
{
	size_t dataLength = w * Color::Length;
	uint8_t* buf = spiDma.GetBuffer();
	for (size_t i = 0; i < dataLength; i += Color::Length) { memcpy(&buf[i], color.Bytes, Color::Length); }

	BeginSendGRamData(x, y, w, h);
	for (int16_t i = 0; i < h; i++) { SendGRamData(buf, dataLength); }
	EndSendGRamData();
}

//点を描画する
void	GLcdGraphics::DrawPixel(int16_t x, int16_t y, const Color& color) const
{
	FillRect(x, y, 1, 1, color);
}

//直線を描画する
//引数：	始点・終点座標 (x1,y1)-(x2,y2)
void	GLcdGraphics::DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const Color& color) const
{
	//垂直線の場合（「始点=終点」の場合もここに引っかかる）
	if (x1 == x2) { DrawLineV(x1, y1, y2, color);	return; }
	
	//水平線の場合
	if (y1 == y2) { DrawLineH(x1, y1, x2, color);	return; }
	
	//計算の都合上、始点座標の右側に終点座標があるようにする(x1 <= x2)。上下の位置関係は不問
	if (x1 > x2) { SwapPoint(&x1, &y1, &x2, &y2); }
	
	int16_t	lengthX, lengthY;	//X方向、Y方向の増加量・距離
	int8_t	stepX, stepY;		//進行方向（右方向が +X方向）
	
	//x軸方向の増加量と進行方向（右方向が +X方向）
	lengthX = x2 - x1;
	stepX = 1;
	
	//y軸方向の増加量と進行方向（下方向が +Y方向）
	if (y1 < y2)
	{
		lengthY = y2 - y1;
		stepY = 1;
	}
	else
	{
		lengthY = y1 - y2;
		stepY = -1;
	}
	
	//直線の傾きが、+-45度以内：		始点からX軸方向に1ドットずつ移動し、累積誤差によりY座標を増（減）させる
	//				+-45度より大きい：	始点からY軸方向に1ドットずつ移動し、累積誤差によりX座標を増（減）させる
	//始点・終点から中点へ向かって点を打つ
	if (lengthY <= lengthX)
	{
		float	a = (float)lengthY / lengthX;	//傾きa
		float	accErr = 0.0f;	//累積誤差
		lengthX++;	lengthY++;	//増加量を座標の距離へ換算
		
		for (uint16_t i = 0; i < lengthX / 2; i++)
		{
			DrawPixel(x1, y1, color);
			DrawPixel(x2, y2, color);
			
			x1 += stepX;
			x2 -= stepX;
			accErr += a;
			
			//Y方向の累積誤差が0.5ドット以上になるとY座標が1ドット変化する
			if (0.5f <= accErr)
			{
				y1 += stepY;
				y2 -= stepY;
				accErr -= 1.0f;	//誤差の値を1ドット分減らす（誤差の許容範囲は-0.5～+0.5ドットの幅1ドット分）
			}
		}
	}
	else
	{
		float	a = (float)lengthX / lengthY;	//傾きaの逆数（傾きはxの変化量1に対するyの変化量を表すので、yの変化量が1のとき、xの変化量は1/a）
		float	accErr = 0.0f;	//累積誤差
		lengthX++;	lengthY++;	//増加量を座標の距離へ換算
		
		for (uint16_t i = 0; i < lengthY / 2; i++)
		{
			DrawPixel(x1, y1, color);
			DrawPixel(x2, y2, color);
			
			y1 += stepY;
			y2 -= stepY;
			accErr += a;
			
			if (0.5f <= accErr)
			{
				x1 += stepX;
				x2 -= stepX;
				accErr -= 1.0f;
			}
		}
	}
	
	//X座標の距離またはY座標の距離が奇数のとき、中点を描く
	if ((lengthX & 0x0001) || (lengthY & 0x0001))
	{
		DrawPixel(x1, y1, color);
	}
}

//水平線を描画する
void	GLcdGraphics::DrawLineH(int16_t x1, int16_t y1, int16_t x2, const Color& color) const
{
	if (x1 > x2) { std::swap(x1, x2); }	//x1よりx2を大きい側にする
	FillRect(x1, y1, x2 - x1 + 1, 1, color);
}

//垂直線を描画する
void	GLcdGraphics::DrawLineV(int16_t x1, int16_t y1, int16_t y2, const Color& color) const
{
	if (y1 > y2) { std::swap(y1, y2); }	//y1よりy2を大きい側にする
	FillRect(x1, y1, 1, y2 - y1 + 1, color);
}

//2点の座標を入れ替える
//・(x1,y1)-(x2,y2)を(x2,y2)-(x1,y1)にする。
void	GLcdGraphics::SwapPoint(int16_t* x1, int16_t* y1, int16_t* x2, int16_t* y2) const
{
	std::swap(x1, x2);
	std::swap(y1, y2);
}

//矩形を描く
void	GLcdGraphics::DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, const Color& color) const
{
	int16_t	x2 = x + w - 1,	y2 = y + h - 1;	//(x,y)の対角位置の座標
	DrawLineH(x, y, x2, color);		//上辺
	DrawLineH(x, y2, x2, color);	//下辺
	DrawLineV(x, y, y2, color);		//左辺
	DrawLineV(x2, y, y2, color);	//右辺
}

//矩形を描く（太さ指定）
void	GLcdGraphics::DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t bold, const Color& color) const
{
	int16_t	x2 = x + w - 1,	y2 = y + h - 1;	//(x,y)の対角位置の座標
	FillRect(x, y,             w, bold, color);	//上辺
	FillRect(x, y2 - bold + 1, w, bold, color);	//下辺
	FillRect(x,             y + bold, bold, h - 2 * bold, color);	//左辺
	FillRect(x2 - bold + 1, y + bold, bold, h - 2 * bold, color);	//右辺
}

//三角形を描く
//引数:	頂点の座標(x1,y1)、(x2,y2)、(x3,y3)
void	GLcdGraphics::DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const Color& color) const
{
	DrawLine(x1, y1, x2, y2, color);
	DrawLine(x2, y2, x3, y3, color);
	DrawLine(x3, y3, x1, y1, color);
}

//塗りつぶした三角形を描く
//引数:	頂点の座標(x1,y1)、(x2,y2)、(x3,y3)
void	GLcdGraphics::FillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const Color& color) const
{
	//x座標の小さい順にソートする
	if (x1 > x2) { SwapPoint(&x1, &y1, &x2, &y2); }	//x1とx2で大きい方をp2へ
	if (x2 > x3) { SwapPoint(&x2, &y2, &x3, &y3); }	//x2とx3で大きい方をp3へ。この時点でp3のx座標が最大
	if (x1 > x2) { SwapPoint(&x1, &y1, &x2, &y2); }	//x1とx2で大きい方をp2へ。この時点でp1のx座標が最小
	
	//三角形を塗りつぶす
	/*
	例A					例B					例C
	|　　　・p2　　|    |　　　・p2　　|    |p1・=p4　　 |
	|p1・　：　　　|    |p1・　：　　　|    |　：　　・p3|
	|　　　p4　・p3| or |　　　・p3=p4 | or |p2・　　　　|
	
	考え方：	三角形を2つに分けて塗りつぶす。
	(1) p2からの垂線と辺p1-p3との交点をp4とする（x1=x2ならp1=p4、x2=x3ならp3=p4、となる）。
	(2) 三角形p1-p2-p3を辺p2-p4で2分する。
	(3) 辺p1-p2と辺p1-p4で作る三角形（ただしp1のみが左端にあるとする）を塗る。→例Aの左側、例Bは結果的に全体、例Cは該当なし。
	(4) 辺p2-p3と辺p4-p3で作る三角形（ただしp3のみが右端にあるとする）を塗る。→例Aの右側、例Bは該当なし、例Cは結果的に全体。
	*/
	if (x1 < x2) { FillBetweenLine(x1, y1, x2, y2, x1, y1, x3, y3, color); }	//'＜'の形（上記(3)の形）
	if (x2 < x3) { FillBetweenLine(x1, y1, x3, y3, x2, y2, x3, y3, color); }	//'＞'の形（上記(4)の形）
}

//2直線L1:(x1s,y1s)-(x1e,y1e)とL2:(x2s,y2s)-(x2e,y2e)に挟まれた領域を塗りつぶす
/*
・L1,L2の共通のX区間を塗りつぶす。
・L1,L2が交わっていても構わない。
・L1,L2とも垂直線ではないとする。
・L1,L2とも始点(xNs)が終点(xNe)より左（X座標が小さい）にあるとする。
*/
void	GLcdGraphics::FillBetweenLine(int16_t x1s, int16_t y1s, int16_t x1e, int16_t y1e,
			int16_t x2s, int16_t y2s, int16_t x2e, int16_t y2e, const Color& color) const
{
	/*
	三角形を塗りつぶすアルゴリズム：
	仮に正三角形の頂点を左からA,B,Cとすると（直線ACより上（Y座標が小さい側）の、点A,CのX座標の間に点Bがある三角形）、
	(1) X座標を点Aから点Bまで1ずつ増加させ、Y座標が「直線AB以上・直線AC以下」の区間に垂直線を引く。
	(2) X座標を点Bから点Cまで1ずつ増加させ、Y座標が「直線BC以上・直線AC以下」の区間に垂直線を引く。
	※右方向が +X方向、下方向が +Y方向。
	*/
	
	//Y座標の小数の切り上げ／切り捨てを簡単にするため、図形を0<=yの領域へ平行移動する
	//・負数の切り上げ／切り捨てを考えたくない。
	int16_t offsetY = y1s;
	if (y1e < offsetY) { offsetY = y1e; }
	if (y2s < offsetY) { offsetY = y2s; }
	if (y2e < offsetY) { offsetY = y2e; }
	y1s -= offsetY;
	y1e -= offsetY;
	y2s -= offsetY;
	y2e -= offsetY;
	
	//直線L1,L2の傾き
	float a1 = (float)(y1e - y1s) / (x1e - x1s);
	float a2 = (float)(y2e - y2s) / (x2e - x2s);
	
	//描画区間
	int16_t startX = std::max(x1s, x2s);
	int16_t endX = std::min(x1e, x2e);
	
	float y1f, y2f;		//直線L1,L2の計算上のY座標
	int16_t yHi, yLo;	//Y座標の上下位置を考慮して整数値に丸めた座標（描画用の座標）
	
	for (int16_t x = startX; x < endX; x++)
	{
		y1f = a1 * (x - x1s) + y1s;
		y2f = a2 * (x - x2s) + y2s;
		
		if (y1f < y2f)
		{
			//直線L1がL2より上にある（座標が小さい側はL1）
			yHi = (int16_t)(y1f);			//切り捨て（上側の線に寄せた点を取る）
			yLo = (int16_t)(y2f + 0.9f);	//切り上げ（下側の線に寄せた点を取る）
		}
		else
		{
			//直線L2がL1より上にある（座標が小さい側はL2）
			yHi = (int16_t)(y2f);			//切り捨て（上側の線に寄せた点を取る）
			yLo = (int16_t)(y1f + 0.9f);	//切り上げ（下側の線に寄せた点を取る）
		}
		
		DrawLineV(x, yHi + offsetY, yLo + offsetY, color);
	}
}

//円を描く
//引数:	中心座標、半径
void	GLcdGraphics::DrawCircle(int16_t cx, int16_t cy, int16_t r, const Color& color) const
{
	DrawCircleWithFillFlag(cx, cy, r, 0, color);
}

//塗りつぶした円を描く
//引数:	中心座標、半径
void	GLcdGraphics::FillCircle(int16_t cx, int16_t cy, int16_t r, const Color& color) const
{
	DrawCircleWithFillFlag(cx, cy, r, 1, color);
}

//円を描く
//引数:	中心座標、半径、塗りつぶしフラグ
void	GLcdGraphics::DrawCircleWithFillFlag(int16_t cx, int16_t cy, int16_t r, uint8_t isFill, const Color& color) const
{
	/*
	弧を描くアルゴリズム：
	描画座標系において、半径rの円の中心から+x方向を0度とし、-45度までの8分円（弧）を考える。
	今、点(r, 0)から出発し、-45度の直線に乗るまで時計回りに次の点を打って行くとする。
	(1) ある点(x, y)を打ったとき、次の点は真下B(x, y+1)か左下L(x-1, y+1)である。
	(2) 点B,点Lそれぞれの座標が計算上の弧からどれくらい距離があるかを求める。これを誤差とする。err = X^2 + Y^2 - R^2
	(3) 誤差B,誤差Lの小さい方の描画点を採択し、点を打つ。
	(4) 点を打つごとに、xの値は同じか減り、yの値は増える。y=x（-45度の直線上）となるまで(1)へ戻る。
	*/
	
	if (r <= 0) { return; }
	
	int16_t	px = r, py = 0;
	
	while (py <= px)
	{
		DrawPixel(cx + px, cy + py, color);	//4時の方向
		DrawPixel(cx - px, cy + py, color);	//8時
		DrawPixel(cx + px, cy - py, color);	//2時
		DrawPixel(cx - px, cy - py, color);	//10時
		DrawPixel(cx + py, cy + px, color);	//1時
		DrawPixel(cx - py, cy + px, color);	//11時
		DrawPixel(cx + py, cy - px, color);	//5時
		DrawPixel(cx - py, cy - px, color);	//7時
		
		if (isFill)
		{
			DrawLineH(cx - py, cy + px, cx + py, color);	//11時-1時
			DrawLineH(cx - px, cy - py, cx + px, color);	//10時-2時
			DrawLineH(cx - px, cy + py, cx + px, color);	//8時-4時
			DrawLineH(cx - py, cy - px, cx + py, color);	//7時-5時
		}
		
		//次の座標候補 Bottom(x, y+1) と LeftBottom(x-1, y+1) について誤差を求める
		py++;
		int16_t s = py * py - r * r;	//「err = X^2 + Y^2 - R^2」の後半2項
		int16_t errB = px * px + s;
		errB = std::abs(errB);
		int16_t errL = (px - 1) * (px - 1) + s;
		errL = std::abs(errL);
		if (errL < errB) { px--; }
	}
}
