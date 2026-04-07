//	PotMeterに長押し判定を追加する／PotMeterをHwSwitch化する
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Axis.hpp"

//初期化
void	Axis::Initialize(gpio_num_t analogPin)
{
	PotMeter::Initialize(analogPin);

	longHoldThresholdTime = UINT32_MAX;
	holdStartTime = 0;
	keyRange = PotMeter::State::MidRange;
	keyState = HwSwitch::State::Free;
}

//状態更新
//・holdStartTime,keyRange,keyStateが更新される。
int16_t	Axis::UpdateState()
{
	auto adcVal = PotMeter::UpdateState();

	using AState = PotMeter::State;
	using DState = HwSwitch::State;

	auto prevKeyRange = keyRange;
	auto prevKeyState = keyState;
	if (keyState == DState::Press) { keyState = DState::Holding; } 
	if (keyState == DState::Release) { keyState = DState::Free; } 

	auto currentAState = PotMeter::GetState();
	if (currentAState == AState::MidRange) { keyRange = AState::MidRange; keyState = DState::Free; }
	if (currentAState == AState::OnRiseToHigh) { keyRange = AState::HighRange; keyState = DState::Press; }
	if (currentAState == AState::OnFallToLow) { keyRange = AState::LowRange; keyState = DState::Press; }
	if (currentAState == AState::OnFallFromHigh) { keyRange = AState::MidRange; keyState = DState::Release; }
	if (currentAState == AState::OnRiseFromLow) { keyRange = AState::MidRange; keyState = DState::Release; }
	if (currentAState == AState::HighRange) { keyRange = AState::HighRange; }	//keyStateはHolding,LongPress,LongHoldingのどれか
	if (currentAState == AState::LowRange) { keyRange = AState::LowRange; }	//keyStateはHolding,LongPress,LongHoldingのどれか

	if (keyState == DState::Free) { return adcVal; }
	if (keyState == DState::Press) { holdStartTime = millis(); return adcVal; }
	if (keyState == DState::Release) { holdStartTime = 0; return adcVal; }

	//この時点でkeyStateはHolding,LongPress,LongHoldingのどれか。prevKeyStateにより決まる
	auto nextKeyState = DState::Holding;	//一旦Holdingとし、条件によりLongPress,LongHoldingに変更する
	auto IsOn = [](DState dState){ return !((dState == DState::Release) || (dState == DState::Free)); };
	if (IsOn(prevKeyState) && IsOn(keyState))	//押し続けている状態
	{
		if (longHoldThresholdTime <= millis() - holdStartTime)
		{
			nextKeyState = (prevKeyState == DState::Holding) ? DState::LongPress : DState::LongHolding;
		}
	}
	keyState = nextKeyState;	//現在のkeyStateが確定した
//Serial.printf(keyState == DState::LongPress ? "LongPress " : "");

	return adcVal;
}
