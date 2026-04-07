//	矩形領域クラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Rectangle.hpp"

//この四角形と別の四角形領域rectとで領域が一致しているか
bool	Rectangle::Equals(Rectangle& rect) const
{
	return (this->x == rect.x) && (this->y == rect.y)
		&& (this->width == rect.width) && (this->height == rect.height);
}

//この四角形が別の四角形領域rectを完全に含んでいるかどうか
bool	Rectangle::Contains(Rectangle& rect) const
{
	return	(this->Left() <= rect.Left()) && (rect.Right() <= this->Right())
		&&	(this->Top() <= rect.Top()) && (rect.Bottom() <= this->Bottom());
}

//この四角形がtargetと交差するか
bool	Rectangle::IntersectsWith(const Rectangle& target) const
{
	if (target.Bottom() < this->Top()) { return false; }	//targetがこの四角形より上にいる
	if (this->Bottom() < target.Top()) { return false; }	//targetがこの四角形より下にいる
	if (target.Right() < this->Left()) { return false; }	//targetがこの四角形より左にいる
	if (this->Right() < target.Left()) { return false; }	//targetがこの四角形より右にいる
	return true;
}

//この四角形とtargetの交差部分を表すRectangleを返す
Rectangle	Rectangle::Intersect(const Rectangle& target) const
{
	int16_t left = std::max(this->Left(), target.Left());
	int16_t right = std::min(this->Right(), target.Right());
	int16_t top = std::max(this->Top(), target.Top());
	int16_t bottom = std::min(this->Bottom(), target.Bottom());

	return Rectangle::FromLTRB(left, top, right, bottom);
}
