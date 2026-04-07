
#include <Arduino.h>
#include "GLcdImage.hpp"
#include "SpiDma.hpp"

//画像を描く
//引数	x,y:	画像の描画先の座標（画面外の座標も可）
//		img:	画像オブジェクト
void	GLcdImage::DrawImage(int16_t x, int16_t y, const Image& img)
{
	Rectangle imgRect(0, 0, img.Width(), img.Height());
	DrawImage(x, y, img, imgRect);
}

//画像を描く
//引数	x,y:	画像の描画先の座標（画面外の座標も可）
//		img:	画像オブジェクト
//	imgRect:	img内の矩形領域
void	GLcdImage::DrawImage(int16_t x, int16_t y, const Image& img, const Rectangle& imgRect)
{
	Rectangle parentRect(0, 0, Width(), Height());
	Rectangle childRect = GetClippedRect(parentRect, x, y, imgRect);	//parentRectは書き換えられる
	if (!childRect.IsValid()) { return; }

	//Strided
	const uint8_t* imgBuf = img.GetBuffer(childRect.X(), childRect.Y());
	size_t imgWidthLength = img.DataLengthOf(img.Width());	//stepLength
	size_t rectWidthLength = img.DataLengthOf(childRect.Width());	//copyLength
	int16_t rectHeight = childRect.Height();	//repeatMax
	uint8_t* dmaBuf = spiDma.GetBuffer();
	imgBufRead.SetStrided(dmaBuf, spiDma.BufferSize(), imgBuf, imgWidthLength, rectWidthLength, rectHeight);

	BeginSendGRamData(parentRect.X(), parentRect.Y(), parentRect.Width(), parentRect.Height());
	while (size_t dataLength = imgBufRead.Next())	{ SendGRamData(dmaBuf, dataLength); }
	EndSendGRamData();
}

//四角形(parent)の(x,y)の位置に置いた別の四角形(child)との交差部分を表す四角形を返す
//引数:
//	parent:	childを乗せようとする四角形領域(0,0,width,height)。　※戻り値として書き換えられる
//	x,y: 	childの左上隅を乗せるparent上の座標。負数やwidth,heightをはみ出す範囲も指定可。
//	child:	parentに乗せようとするchild内の領域。child座標系。
//戻り値:	結果の領域（child内の領域）をchild座標系で表したもの
//戻り値(引数parent):	結果の領域をparent座標系で表したもの
//注意:	交差部分がなかった場合、戻り値のRectangleはどちらもIsValid()==false.
Rectangle	GLcdImage::GetClippedRect(Rectangle& parent, int16_t parentX, int16_t parentY, const Rectangle& child) const
{
	Rectangle childOnParent(parentX, parentY, child.Width(), child.Height());
	Rectangle intersectInParent = parent.Intersect(childOnParent);	//parent座標系で表された交差部分

	Rectangle vchild(0, 0, child.Width(), child.Height());	//child(x,y)を原点と見た仮のchild座標系へchildを一旦配置する（以下、vchild座標系, vchildと呼ぶ）
	Rectangle parentOnChild(-parentX, -parentY, parent.Width(), parent.Height());	//vchild座標系に配置したparent矩形
	Rectangle intersectInChild = vchild.Intersect(parentOnChild);	//parentとvchildの重なった部分（vchild座標系）
	intersectInChild.Offset(child.X(), child.Y());	//vchild座標系にある矩形を元のchild座標系に配置する

	parent = intersectInParent;
	return intersectInChild;
}

/*	説明：	矩形領域の交差部分の求め方

■矩形領域の交差を求める関数Func(px, py, child(cx,cy,cw,ch))の様子
p(0,0)
+-------------+  <--parent矩形
|    c(0,0)   |
|    +----------------+  <--child矩形
|    |        :       |
|    |    c(cx,cy)=p(px,py)
|    |    +===-----+  |  <--childの特定領域c(cx,cy,cw,ch)
|    |    |///:    |  |		※求める交差部分
|    |    |///:    |  |
|    |    +===-----+  |
|    |        :       |
|    +----------------+
|             |
+-------------+

■child座標系をvchild座標系に変換するとこうなる（v:仮想的な）
■child座標系のまま交差部分を求めるのは面倒だが、vchild座標系の形なら容易に求まる
p(0,0)
+-------------+  <--parent矩形
|    c(0,0)   |
|    +--      |
|    |        |
|         c(cx,cy)=vc(0,0)
|         +===-----+-------+  <--vchild矩形
|         |///:    |       |	※c(cx,cy)を原点とした仮child矩形vc(0,0,cw,ch)
|         |///:    |       |	※交差部分はvc(0,0,cw',ch')となる
|         +===-----+       |
|         |   :            |
|         |   :            |
|         |   :            |
+---------|---+            |
          +----------------+

■vchild矩形を(-cx,-cy)オフセットするとchild座標系での表現に戻る
■交差部分の表現はvc(0,0,cw',ch')→c(cx,cy,cw',ch')となる
■c(cx,cy,cw',ch')がchild座標系での答え
p(0,0)
+-----
|    vc(-cx,-cy)=c(0,0)
|    +----------------+  <--child矩形
     |                |
     |    vc(0,0)=c(cx,cy)
     |    +===-----+--|----  <--vchild矩形
     |    |///:    |  |
     |    |///:    |  |
     |    +===-----+  |
     |    |           |
     +----------------+
          |
 
■parentとchild（の特定領域）の重なり方には下記のようなパターンもあるが、交差部分の求め方は同じ
■結果はc(cx',cy',cw',ch')となる
              p(0,0)
              +-------------+  <--parent矩形
     c(0,0)   |             |
     +----------------+  <--child矩形
     |        :       |     |
     |    c(cx,cy)=p(px,py) |  <--座標pxは負数
     |    +----====+  |
     |    |   :////|  |     |
     |    |   :////|  |     |
     |    +----====+  |     |
     |        :       |     |
     +----------------+     |
              |             |
              +-------------+
 */
