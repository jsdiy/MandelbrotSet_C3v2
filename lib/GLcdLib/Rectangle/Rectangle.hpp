//	矩形領域クラス, 縦横サイズ構造体, 座標構造体
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/04	初版（旧GraphicsLib版）
//	2026/01	全面改訂

#pragma	once

#include <Arduino.h>

class	Rectangle
{
private:
	int16_t	x, y, width, height;

public:
	Rectangle(int16_t x, int16_t y, int16_t w, int16_t h) : x(x), y(y), width(w), height(h) {}
	static	Rectangle	FromLTRB(int16_t left, int16_t top, int16_t right, int16_t bottom)
		{ return Rectangle(left, top, right - left + 1, bottom - top + 1); }

	int16_t	X() const { return x; }
	int16_t	Y() const { return y; }
	int16_t	Width() const { return width; }
	int16_t	Height() const { return height; }
	int16_t	Left() const { return x; }
	int16_t	Top() const { return y; }
	int16_t	Right() const { return x + width - 1; }
	int16_t	Bottom() const { return y + height - 1; }
	bool	IsValid() const { return (0 < width) && (0 < height); }
	bool	Equals(Rectangle& rect) const;
	bool	Contains(Rectangle& rect) const;

	Rectangle	OffsetBy(int16_t x, int16_t y) const	//この四角形を移動させた四角形を作成する
		{ return Rectangle(this->x + x, this->y + y, this->width, this->height); }
	void	Offset(int16_t x, int16_t y) { this->x += x; this->y += y; }	//この四角形の位置を指定した量だけ移動する
	bool	IntersectsWith(const Rectangle& target) const;	//この四角形がtargetと交差するか
	Rectangle	Intersect(const Rectangle& target) const;	//この四角形とtargetの交差部分を表すRectangleを返す

	//関数名のヒント	https://learn.microsoft.com/ja-jp/dotnet/api/system.drawing.rectangle?view=net-8.0
};

struct	RectSize
{
	int16_t	Width, Height;
	RectSize(int16_t width, int16_t height) : Width(width), Height(height) {}
	bool	IsValid() const { return (0 < Width) && (0 < Height); }
};

struct	Point
{
	int16_t	X, Y;
	Point(int16_t x, int16_t y) : X(x), Y(y) {}
};
