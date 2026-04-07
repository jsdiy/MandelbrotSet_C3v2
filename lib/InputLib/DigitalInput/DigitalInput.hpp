//	デジタル入力	チャタリング対策あり
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
/*
2025/01	AVR版(2019/01)を移植、ReleaseとOffの意味を入れ替え
2025/09	ESP32版としてmills()利用へ改造
2025/10	Clicked()などの状態を追加。利便性向上
2026/03	HwSwitchをDigitalInputとHwSwitchに分割（AnalogInput,PotMeterに合わせた）
*/

#pragma	once

#include <Arduino.h>

//push_switch, toggle_switch, slide_switch, ...
class	DigitalInput
{
protected:
	static	constexpr	ulong	DebounceTimeMillis = 20;	//チャタリング継続時間（想定）

private:
	gpio_num_t	pin;
	int8_t	pinVal;
	ulong	debounceTime = DebounceTimeMillis, prevTime = 0;

public:
	DigitalInput() {}
	void	Initialize(gpio_num_t swPin);
	void	SetDebounceTime(ulong millis) { debounceTime = millis; }
	int8_t	Read();
};
