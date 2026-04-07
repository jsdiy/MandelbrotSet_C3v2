#pragma	once

#include <Arduino.h>
#include "IGLcd.hpp"
#include "ByteReader.hpp"
#include "Image.hpp"
#include "Rectangle.hpp"

class	GLcdImage	: public virtual IGLcd
{
private:
	ByteReader	imgBufRead;
	Rectangle	GetClippedRect(Rectangle& parent, int16_t parentX, int16_t parentY, const Rectangle& child) const;

public:
	void	DrawImage(int16_t x, int16_t y, const Image& img);
	void	DrawImage(int16_t x, int16_t y, const Image& img, const Rectangle& imgRect);

protected:
	virtual	~GLcdImage() = default;
};
