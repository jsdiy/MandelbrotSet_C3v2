//	アナログ入力
//	potentio_meter, slid_volune, stick/lever, ...
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "PotMeter.hpp"

void	PotMeter::Initialize(gpio_num_t analogPin)
{
	AnalogInput::Initialize(analogPin);
	currentState = State::MidRange;
	SetInverted(0);
	SetRisingThreshold(INT16_MAX, 0);
	SetFallingThreshold(INT16_MIN, 0);
}

void	PotMeter::SetThreshold(Threshold& obj, int16_t threshold, int16_t hysteresis)
{
	obj.threshold = threshold;
	obj.hysteresis = hysteresis;
}

PotMeter::State	PotMeter::GetNextState(int16_t curentAdcVal)
{
	const auto MidRangeUpper = thrRise.threshold;
	const auto HighRangeLower = thrRise.threshold - thrRise.hysteresis;
	const auto MidRangeLower = thrFall.threshold;
	const auto LowRangeUpper = thrFall.threshold + thrFall.hysteresis;

	if (currentState == State::OnRiseToHigh) { currentState = State::HighRange; }
	if (currentState == State::OnFallFromHigh) { currentState = State::MidRange; }
	if (currentState == State::OnFallToLow) { currentState = State::LowRange; }
	if (currentState == State::OnRiseFromLow) { currentState = State::MidRange; }
	State nextState = currentState;

	if (currentState == State::HighRange)
	{
		if (curentAdcVal < HighRangeLower) { nextState = State::OnFallFromHigh; }
	}
	else if (currentState == State::LowRange)
	{
		if (LowRangeUpper < curentAdcVal) { nextState = State::OnRiseFromLow; }
	}
	else	//(currentState == State::MidRange)
	{
		if (MidRangeUpper <= curentAdcVal) { nextState = State::OnRiseToHigh; }
		else if (curentAdcVal <= MidRangeLower) { nextState = State::OnFallToLow; }
	}

	return nextState;
}

//状態を更新する
//・currentStateを更新する。
//・戻り値は現在のADC値。
int16_t	PotMeter::UpdateState()
{
	int16_t curentAdcVal = Read();
	if (0 < complementAdcVal) { curentAdcVal = complementAdcVal - curentAdcVal; }
	currentState = GetNextState(curentAdcVal);
	return curentAdcVal;
}
