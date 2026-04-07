//	アナログ入力	平滑化対応
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma once

#include <Arduino.h>

//potentio_meter, slid_volune, stick/lever, ...
class	AnalogInput
{
protected:
	static	constexpr	uint8_t	SamplingCount		= 4;	//サンプリング回数（目安）
	static	constexpr	uint8_t	SamplingCountMax	= 10;	//その上限（無駄に多くする必要はない）

private:
	gpio_num_t	pin;
	uint8_t	sampling = SamplingCount;
	uint32_t (*FnAnalogRead)(uint8_t);	//analogRead()とanalogReadMilliVolts()の入れ替え

public:
	AnalogInput() {}
	void	Initialize(gpio_num_t analogPin);
	void	SetOverSampling(uint8_t samplingCount);
	void	MilliVoltMode(bool isOn);
	int16_t	Read() const;	//ADC:0-4095, 電圧表現:0-3300[mV] ※減衰比11dB,基準1.1Vでの理論値
};
