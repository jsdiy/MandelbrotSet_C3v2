//アプリケーション本体

#include "Application.hpp"

void	Application::Setup(GLcd& lcd, Joystick& jstk, Color& foreColor, Color& bgColor, Color& colBreak)
{
	this->lcd = &lcd;
	this->foreColor = &foreColor;
	this->bgColor = &bgColor;
	this->colBreak = &colBreak;
	this->jstk = &jstk;

	using Key = Joystick::KeyCode;
	jstk.OnPressCb().Add(Key::Up, [this](){ OnKeyPressCb(Key::Up); });
	jstk.OnPressCb().Add(Key::Down, [this](){ OnKeyPressCb(Key::Down); });
	jstk.OnPressCb().Add(Key::P, [this](){ OnKeyPressCb(Key::P); });

	opMode = OpMode::Drawing;
	isOpModeFirstLoop = false;
	
	mandelbrot.Initialize(lcd.Width(), lcd.Height());
	mandelbrot.SetPixelHandler([this](int16_t idx, uint16_t* buf, size_t len){ DrawPixel(idx, buf, len); });
	CreateInfoImage();
}

void	Application::Loop()
{
	bool isUpdated = jstk->CheckKeyState();
	if (!isUpdated) { return; }

	OpMode prevOpMode = opMode;
	switch (opMode)
	{
	case	OpMode::Drawing:		opMode = Drawing();	break;
	case	OpMode::DrawBreak:		opMode = DrawBreak();	break;
	case	OpMode::DrawComplete:	opMode = DrawComplete();	break;
	case	OpMode::DrawSetting:	opMode = DrawSetting();	break;
	default:	break;
	}
	isOpModeFirstLoop = (prevOpMode != opMode);
}

//キー入力による非同期処理
//・Joystickからイベントを取得するのではなく、直前に設定したkeyBitsに対してキー状態を判定する必要がある。
//	→非同期処理なので。また、同時押しがある場合を考慮するとそういうことになる。
void	Application::OnKeyPressCb(Joystick::KeyCode keys)
{
	SaveInputKey(keys);
	if (opMode == OpMode::Drawing)
	{
		using Key = Joystick::KeyCode;
		if (IsInputKey(Key::Up | Key::Down | Key::P)) { mandelbrot.StopCalculation(); }
	}
}

//描画中
OpMode	Application::Drawing()
{
	jstk->KeyEventTrigger(true);	//キー操作の非同期処理を開始
	bool isComplete = mandelbrot.StartCalculation();	//false:描画が完了する前に中断された
	jstk->KeyEventTrigger(false);	//停止
	return isComplete ? OpMode::DrawComplete : OpMode::DrawBreak;
}

OpMode	Application::DrawBreak()
{
	using Key = Joystick::KeyCode;
	OpMode	nextOpMode = OpMode::DrawComplete;

	//拡大設定して新規描画開始
	if (IsInputKey(Key::Up))
	{
		mandelbrot.ZoomIn();
		nextOpMode = OpMode::Drawing;
	}
	//縮小設定して新規描画開始
	else if (IsInputKey(Key::Down))
	{
		mandelbrot.ZoomOut();
		nextOpMode = OpMode::Drawing;
	}
	//モード切替
	else if (IsInputKey(Key::P))
	{
		nextOpMode = OpMode::DrawSetting;
	}

	return nextOpMode;
}

//描画が完了した／操作待ち
OpMode	Application::DrawComplete()
{
	using Key = Joystick::KeyCode;
	OpMode	nextOpMode = OpMode::DrawComplete;

	//斜め押しの場合に左右動作を優先させたいので、左右判定を上下判定より先に持ってくる
	//情報表示
	if (jstk->OnKeyPress(Key::Left))
	{
		ShowInformation();
	}
	//情報非表示
	else if (jstk->OnKeyPress(Key::Right))
	{
		HideInformation();
	}
	//拡大設定して新規描画開始
	else if (jstk->OnKeyPress(Key::Up))
	{
		mandelbrot.ZoomIn();
		nextOpMode = OpMode::Drawing;
	}
	//縮小設定して新規描画開始
	else if (jstk->OnKeyPress(Key::Down))
	{
		mandelbrot.ZoomOut();
		nextOpMode = OpMode::Drawing;
	}
	//モード切替
	else if (jstk->OnKeyPress(Key::P))
	{
		nextOpMode = OpMode::DrawSetting;
	}

	return nextOpMode;
}

OpMode	Application::DrawSetting()
{
	if (isOpModeFirstLoop) { DrawSettingReady(); }
	using Key = Joystick::KeyCode;

	if (jstk->IsKeyHolding(Key::Right)) { MoveCursor(Key::Right); isCursorMove = true; }
	else if (jstk->IsKeyHolding(Key::Left)) { MoveCursor(Key::Left); isCursorMove = true; }
	else if (jstk->IsKeyHolding(Key::Up)) { MoveCursor(Key::Up); isCursorMove = true; }
	else if (jstk->IsKeyHolding(Key::Down)) { MoveCursor(Key::Down); isCursorMove = true; }
	else if (jstk->OnKeyPress(Key::P))
	{
		if (isCursorMove) { mandelbrot.SetFocus(cursorX, cursorY); }
		//↑CenterCursor()で示される画面座標(x,y)は実際の中心座標から逆算したものではないので、
		//SetFocus()すると中心座標がずれる。よって、カーソル移動がなかった場合はSetFocus()しないようにする。
		return OpMode::Drawing;
	}

	return OpMode::DrawSetting;
}

void	Application::DrawSettingReady()
{
	CenterCursor();
	DrawCursor(foreColor);
	DisplaySettingModeFrame(foreColor);
	isCursorMove = false;
}

void	Application::MoveCursor(Joystick::KeyCode direc)
{
	if (!IsCursorMoveTiming()) { return; }
	using Key = Joystick::KeyCode;

	switch (direc)
	{
	case	Key::Up:	MoveCursorV(-1);	break;
	case	Key::Down:	MoveCursorV(+1);	break;
	case	Key::Left:	MoveCursorH(-1);	break;
	case	Key::Right:	MoveCursorH(+1);	break;
	default:	break;
	}
}

bool Application::IsCursorMoveTiming()
{
	auto currentTime = millis();
	if (CursorSpeed <= currentTime - prevCursorMoveTime)
	{
		prevCursorMoveTime = currentTime;
		return true;
	}
	else
	{
		return false;
	}
}

//操作画面の枠を表示する
void	Application::DisplaySettingModeFrame(Color* color)
{
	uint8_t bold = 2;
	lcd->DrawRect(0, 0, lcd->Width(), lcd->Height(), bold, *color);
}

//カーソル座標を画面中央位置にセットする
void	Application::CenterCursor()
{
	cursorX = lcd->Width() / 2;
	cursorY = lcd->Height() / 2;
}

//カーソルを描く
void	Application::DrawCursor(Color* color)
{
	int8_t bold = 2;
	lcd->FillRect(cursorX, cursorY, bold, bold, *color);
}

//カーソルを水平移動する
void	Application::MoveCursorH(int8_t sign)
{
	DrawCursor(bgColor);
	cursorX += (sign * GridSize);
	if (lcd->Width() - GridSize < cursorX) { cursorX = GridSize; }
	else if (cursorX - GridSize < GridSize) { cursorX = (lcd->Width() - GridSize) / GridSize * GridSize; }
	DrawCursor(foreColor);
}

//カーソルを垂直移動する
void	Application::MoveCursorV(int8_t sign)
{
	DrawCursor(bgColor);
	cursorY += (sign * GridSize);
	if (lcd->Height() - GridSize < cursorY) { cursorY = GridSize; }
	else if (cursorY - GridSize < GridSize) { cursorY = (lcd->Height() - GridSize) / GridSize * GridSize; }
	DrawCursor(foreColor);
}

//描画情報欄のグラフィックバッファを作成する
void	Application::CreateInfoImage()
{
	int16_t cW, cH;	lcd->GetCharSize(&cW, &cH);
	infoX = cW;
	infoW = lcd->Width() - infoX * 2;
	infoH = cH * InfoLineLength;
	if (infoH % MandelbrotSet::CalcStep != 0) { infoH = (infoH / MandelbrotSet::CalcStep + 1) * MandelbrotSet::CalcStep; }
	infoY = (lcd->Height() - infoH) / MandelbrotSet::CalcStep * MandelbrotSet::CalcStep;
	infoY -= MandelbrotSet::CalcStep * 2;	//適当な調整値

	size_t bufSize = lcd->Width() * infoH * Color::Length;	//RGB565=2bytes
	auto imgBuf = (uint8_t*)heap_caps_malloc(bufSize, MALLOC_CAP_32BIT);
	if (!imgBuf) { Serial.println("Application::CreateInfoImage() - heap_caps_malloc() failure."); return; }
	imgInfo.SetImage(EImageFormat::RGB565, lcd->Width(), infoH, imgBuf, bufSize);
	Serial.printf("imgInfo buf: 0x%X, %dbytes. %dlines\n", imgInfo.GetBuffer(), imgInfo.BufLength(), InfoLineLength);
}

//MandelbrotSetからのコールバック
//計算結果を描画する
void	Application::DrawPixel(int16_t lineIndex, uint16_t* iterBuffer, size_t iterBufLength)
{
	auto lineBuffer = spiDma.GetBuffer();
	auto lineBufLength = iterBufLength * sizeof(uint16_t)/*RGB565*/;

	//反復回数を色データに変換する
	for (size_t bidx = 0, i = 0; i < iterBufLength; i++)
	{
		uint16_t color = ToRGB565(iterBuffer[i]);
		lineBuffer[bidx++] = (uint8_t)((color >> 8) & 0xFF);
		lineBuffer[bidx++] = (uint8_t)(color & 0xFF);
	}

	//画面に描画する
	lcd->DrawImage(0, lineIndex, lcd->Width(), MandelbrotSet::CalcStep, lineBuffer, lineBufLength);
	
	//lineBufferを情報欄グラフィックバッファにコピーする
	if (infoY <= lineIndex && lineIndex < infoY + infoH)
	{
		auto imgBuf = imgInfo.GetBuffer(0, lineIndex - infoY);
		memcpy(imgBuf, lineBuffer, lineBufLength);
	}
}

//反復回数を色に変換する
//・8bit値のビット列で0を00へ、1を11へ置換する。0b11001010 --> 0b1111000011001100
uint16_t	Application::ToRGB565(uint16_t iterCount)
{
	uint16_t colorBits = 0;
	if (iterCount < MandelbrotSet::IterMax)
	{
		for (uint8_t wpos = 0, rpos = 0; rpos < 8; rpos++, wpos += 2)
		{
			if (iterCount & (1 << rpos)) { colorBits |= (0b11 << wpos); }
		}
	}
	return colorBits;
}

//描画情報を出力する
void	Application::PrintInfo()
{
	MandelbrotSet::TInfo info = mandelbrot.GetInfo();
	int16_t cW, cH;	lcd->GetCharSize(&cW, &cH);
	int16_t posY = infoY;
	lcd->Printf(infoX, posY, "Lv:%d", info.level);	posY += cH;
	lcd->Printf(infoX, posY, "C (%13.10f,%13.10f)", info.centerX, info.centerY);	posY += cH;
	lcd->Printf(infoX, posY, "Re(%13.10f,%13.10f)", info.reMin, info.reMax);	posY += cH;
	lcd->Printf(infoX, posY, "Im(%13.10f,%13.10f)", info.imMin, info.imMax);
}

//情報欄の背景画像を描画する（描いた情報を消す）
void	Application::DrawInfoBg()
{
	//imgInfoをinfoX,Y,W,Hの領域に描く
	auto rect = Rectangle(infoX, 0, infoW, infoH);	//imgInfo上の対象領域
	lcd->DrawImage(infoX, infoY, imgInfo, rect);
}
