//	アナログ入力 非同期版
//	potentio_meter, slid_volune, stick/lever, ...
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "PotMeterA.hpp"

//イベントの監視を開始する
void	PotMeterA::StartMonitoring(uint32_t tickMillis)
{
	tkPolling.attach_ms(tickMillis, +[](PotMeterA* me){ me->UpdateState(); }, this);
	//↑「+」はキャプチャなし（[]が空文）のラムダを「関数ポインタ」へ明示的に変換する構文
}

//状態を更新する
//・戻り値は現在のADC値。
int16_t	PotMeterA::UpdateState()
{
	int16_t curentAdcVal = PotMeter::UpdateState();
	EventTrigger(curentAdcVal);
	return curentAdcVal;
}

//イベントを実行する
void	PotMeterA::EventTrigger(int16_t curentAdcVal)
{
	auto state = GetState();
	if (state == State::OnRiseToHigh) { onRiseToHigh.Invoke(curentAdcVal); }
	if (state == State::OnFallFromHigh) { onFallFromHigh.Invoke(curentAdcVal); }
	if (state == State::OnFallToLow) { onFallToLow.Invoke(curentAdcVal); }
	if (state == State::OnRiseFromLow) { onRiseFromLow.Invoke(curentAdcVal); }
}
