//	デジタル入力	チャタリング対策あり
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "DigitalInput.hpp"

//初期化
void	DigitalInput::Initialize(gpio_num_t swPin)
{
	this->pin = swPin;
	pinMode(swPin, INPUT_PULLUP);
	pinVal = HIGH;
}

//ピン状態を読み込む
int8_t	DigitalInput::Read()
{
	//チャタリング対策として、チャタリング継続時間より長い間隔でピンを読み込む
	auto nowTime = millis();
	if (debounceTime < nowTime - prevTime)
	{
		pinVal = digitalRead(pin);
		prevTime = nowTime;
	}
	return pinVal;
}

/*	チャタリング対策の説明
チャタリング継続時間より長い間隔でピンを読み込めば、チャタリングの影響を受けない

■チャタリング中ではないときに読み込んだ場合、スイッチ操作前後の状態と矛盾はない
HIGH----------|||||__________LOW
	+		+		+		+
	H		H		L		L

■チャタリング中に読み込んだ場合、それがHIGHでもLOWでもスイッチ操作前後の状態と矛盾はない
HIGH----------|||||__________LOW
		+		+		+
		H		H/L		L
*/
