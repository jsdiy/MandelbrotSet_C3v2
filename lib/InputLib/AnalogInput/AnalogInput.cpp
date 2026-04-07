//	アナログ入力	平滑化対応・ADC値とmV値の切り替え対応
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "AnalogInput.hpp"

void	AnalogInput::Initialize(gpio_num_t analogPin)
{
	this->pin = analogPin;
	MilliVoltMode(false);
}

void	AnalogInput::SetOverSampling(uint8_t samplingCount)
{
	sampling = std::min(samplingCount, SamplingCountMax);
	if (sampling == 0) { sampling = 1; }
}

void	AnalogInput::MilliVoltMode(bool isOn)
{
	FnAnalogRead = isOn ? analogReadMilliVolts : [](uint8_t ap){ return (uint32_t)analogRead(ap); };
}

int16_t	AnalogInput::Read() const
{
	uint32_t val = 0;
	for (uint8_t i = 0; i < sampling; i++) { val += FnAnalogRead(pin); }
	val /= sampling;
	return (int16_t)val;
}
