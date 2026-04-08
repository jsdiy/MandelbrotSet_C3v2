//アプリケーション本体

#pragma once

#include <Arduino.h>
#include "MandelbrotSet.hpp"
#include "GLcd.hpp"
#include "Joystick.hpp"

//OperationMode
enum	class	OpMode	: uint8_t
{
	Drawing,		//描画中
	DrawBreak,		//描画が中断された
	DrawComplete,	//描画が完了した／操作待ち
	DrawSetting,	//描画パラメータ設定モード
};

//メインクラス
class	Application
{
private:
	static	constexpr	int8_t	GridSize = 8;	//カーソル移動格子の大きさ（単位は[画素]）
	static	constexpr	uint32_t	CursorSpeed = 250;	//カーソル移動の速さ(ms)　※何ミリ秒で1グリッド移動するか

private:
	MandelbrotSet	mandelbrot;
	GLcd* lcd = nullptr;
	Joystick*	jstk = nullptr;
	Color	*foreColor = nullptr, *bgColor = nullptr;
	OpMode	opMode;

	int16_t	cursorX, cursorY;	//画面座標
	bool	isCursorMove;
	uint32_t prevCursorMoveTime = 0;
	bool	IsCursorMoveTiming();

	volatile	uint16_t	keyBits = 0;	//キー入力非同期動作中に押されたキーを保存する
	void	SaveInputKey(Joystick::KeyCode keys) { keyBits = static_cast<uint16_t>(keys); }
	bool	IsInputKey(Joystick::KeyCode keys) { return (keyBits & static_cast<uint16_t>(keys)); }
	void	OnKeyPressCb(Joystick::KeyCode keys);

	bool	isOpModeFirstLoop;
	OpMode	Drawing();
	OpMode	DrawBreak();
	OpMode	DrawComplete();
	OpMode	DrawSetting();

	void	DrawSettingReady();
	void	DisplaySettingModeFrame(Color* color);
	void	CenterCursor();
	void	DrawCursor(Color* color);
	void	MoveCursor(Joystick::KeyCode direc);
	void	MoveCursorH(int8_t sign);
	void	MoveCursorV(int8_t sign);
	void	ShowInformation() { PrintInfo(); }
	void	HideInformation() { DrawInfoBg(); }

private:
	static	constexpr	int8_t	InfoLineLength = 4;
	int16_t	infoX, infoY, infoW, infoH;
	Image	imgInfo;
	void	CreateInfoImage();
	void	PrintInfo();
	void	DrawInfoBg();
	void	DrawPixel(int16_t lineIndex, uint16_t* iterBuffer, size_t iterBufLength);
	uint16_t	ToRGB565(uint16_t iterCount);

public:
	Application() {}
	void	Setup(GLcd& lcd, Joystick& jstk, Color& foreColor, Color& bgColor);
	void	Loop();
};
