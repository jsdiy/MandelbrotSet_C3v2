//	ハードウェアスイッチ 非同期版
//	push_switch, toggle_switch, slide_switch, ...
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "HwSwitchA.hpp"

//イベントの監視を開始する
void	HwSwitchA::StartMonitoring(uint32_t tickMillis)
{
	tkPolling.attach_ms(tickMillis, +[](HwSwitchA* me){ me->UpdateState(); }, this);
	//↑「+」はキャプチャなし（[]が空文）のラムダを「関数ポインタ」へ明示的に変換する構文
}

//状態を更新する
//・戻り値は現在のピンの値。
int8_t	HwSwitchA::UpdateState()
{
	int8_t curentPinVal = HwSwitch::UpdateState();
	EventTrigger(curentPinVal);
	return curentPinVal;
}

//イベントを実行する
void	HwSwitchA::EventTrigger(int8_t currentPinVal)
{
	State currentState = GetState();

	//Press発生
	if (currentState == State::Press) { onPress.Invoke(); }

	//LongPress発生
	if (currentState == State::LongPress) { onLongPress.Invoke(); }

	//Release発生
	if (currentState == State::Release) { onRelease.Invoke(); }

	//ピンの状態が変化した: CHANGE発生
	if (currentState == State::Press || currentState == State::Release)
	{
		//・スイッチにActiveLow/ActiveHighの違いがあるのでSwOn/SwOffで判定してはいけない。HIGH/LOWで判定すること。
		//・attachInterrupt()のCHANGEと比べて、RISING/FALLINGのどちらが発生したかを通知できるのがメリット。
		if (currentPinVal == HIGH)
		{
			//LOW→HIGHへ変化した: RISING発生
			onRising.Invoke();
			onChange.Invoke(RISING);
		}
		else
		{
			//HIGH→LOWへ変化した: FALLING発生
			onFalling.Invoke();
			onChange.Invoke(FALLING);
		}
	}
}
