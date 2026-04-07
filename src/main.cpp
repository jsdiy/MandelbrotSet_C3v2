//	マンデルブロ集合
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2025/10	初版
//	2026/02	LCDをST7735からILI9225へ変更, 入力デバイスをボタンからレバー＋ボタンへ変更
//	2026/04	mainをApplicationクラスに分離, MandelbrotSetクラスから描画処理を分離

/*	ピンアサイン
			ESP32-C3
SPI-SS		GPIO_NUM_10
SPI-MOSI	GPIO_NUM_7
SPI-MISO	GPIO_NUM_2	グラフィックLCDでは未使用
SPI-SCK		GPIO_NUM_6
LCD-DC		GPIO_NUM_4
LCD-RESET	GPIO_NUM_NC
SW-A		GPIO_NUM_8
SW-B		GPIO_NUM_9

			ESP32-C3	*…BootStrap関連ピン
			[Antenna side]
			3V3		IO-0	JoystickAxisA
			EN		IO-1	JoystickAxisB
LCD-DC		IO-4	IO-2*	SPI-MISO
			IO-5	IO-3	JoystickPushSw
SPI-SCK		IO-6	IO-19	USB_D+
SPI-MOSI	IO-7	IO-18	USB_D-
SW-A		IO-8*	TXD
SW-B		IO-9*	RXD
			GND		IO-10*	SPI-SS
			[USB-conn side]
*/
/*	レバー/ボタンに割り当てる機能
・拡大/縮小
・中心箇所移動
・[描画]/[操作]モードの切り替え
・情報表示

描画モード
[描画中]
入力があったら描画を停止する
・拡大/縮小：新拡大率で直ちに描画開始：レバー↑↓
・モード切替：描画を中止して操作モードへ：モードボタン

[描画完了／待機時]
・拡大/縮小：新拡大率で直ちに描画開始：レバー↑↓
・情報表示：[倍率・中心座標・区間]を表示／非表示：レバー←/→
・モード切替：操作モードへ：モードボタン

操作モード
・中断枠を表示
[操作中]
・中心箇所移動：カーソルを移動：レバー↑↓←→
・モード切替：描画モード[描画中]へ：モードボタン	※中断枠を消すため再描画する

オプション機能：
・画像保存
	→描画処理の呼び出しごとにファイルへ追記する仕組みを検討する
*/

#include <Arduino.h>
#include "Application.hpp"
#include "LcdILI9225.hpp"
#include "Joystick.hpp"

//オブジェクト
Application	app;
LcdILI9225	lcd;
Joystick	jstk;

//色
static	Color	foreColor(0xFF, 0xFF, 0xFF);
static	Color	bgColor(0x00, 0x00, 0x00);
static	Color	colBreak(0xFF, 0xFF, 0x00);

//定数
static	constexpr	gpio_num_t
	PinX = GPIO_NUM_1, PinY = GPIO_NUM_0, PinSwP = GPIO_NUM_3,
	PinSwA = GPIO_NUM_8, PinSwB = GPIO_NUM_9, PinSwC = GPIO_NUM_NC;

void	setup(void)
{
	Serial.begin(115200);
	delay(2000);

	lcd.Initialize(GPIO_NUM_10, GPIO_NUM_4, 1, 1, 1);
	lcd.RotateFlip(ERotFlip::Rot270);
	lcd.SetTextScale(1, 2);
	lcd.SetTextColor(foreColor, bgColor);
	lcd.ClearScreen(bgColor);

	jstk.Initialize(PinX, PinY, true, true, 3, 8, PinSwP, PinSwA, PinSwB, PinSwC);

	app.Setup(lcd, jstk, foreColor, bgColor, colBreak);
}

void	loop(void)
{
	app.Loop();
}
