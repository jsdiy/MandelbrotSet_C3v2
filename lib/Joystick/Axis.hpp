//	PotMeterに長押し判定を追加する／PotMeterをHwSwitch化する
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma once

#include <Arduino.h>
#include "PotMeter.hpp"
#include "HwSwitch.hpp"	//HwSwitch::Stateを参照する

//ジョイスティックのレバーやゲームパッドの十字ボタンの、上下または左右の入力操作を想定したクラス
class	Axis	: public PotMeter
{
private:
	uint32_t	longHoldThresholdTime, holdStartTime;
	PotMeter::State	keyRange;	//Right/LeftやUp/Downを意味するHighRange/LowRange。またはボタンオフを意味するMidRange。
	HwSwitch::State	keyState;	//ボタン表現での状態／イベント

public:
	Axis() {}
	void	Initialize(gpio_num_t analogPin);	//PotMeterの同名関数を隠蔽
	void	SetLongHoldThresholdTime(uint32_t millis) { longHoldThresholdTime = millis; }
	int16_t	UpdateState();	//PotMeterの同名関数を隠蔽
	HwSwitch::State	GetState(PotMeter::State& retKeyRange) { retKeyRange = keyRange; return keyState; }	//PotMeterの同名関数を隠蔽
};

/*	HwSwitch::State	GetState(PotMeter::State& retKeyRange) の説明
・GetState()の戻り値はHwSwitch::State。ボタン操作と同様に見ることができる。
・PotMeterクラスを十字ボタンと見ると、ADCの中点を境にmax側と0側で2個のボタンが接続された形に相当する。
	どちらのボタンが操作されたかの区別を、引数戻り値(retKeyRange)として返す。
・retKeyRangeはPotMeter::State::HighRange,MidRange,LowRangeを取る。
	関数の戻り値HwSwitch::Stateが、どちらのボタンに対するものかを示している。
		HighRange…戻り値HwSwitch::Stateは、ADCのmax側のボタンに対するものである
		LowRange …戻り値HwSwitch::Stateは、ADCの0側のボタンに対するものである
		MidRange …ボタンが押されていない状態である（放された瞬間も含め）
・Axisクラス自体はX軸かY軸かの区別はないので、呼び出し側でHighRange/LowRangeの意味を解釈する。
	AxisクラスがX軸（十字ボタンの左右）であれば、HighRange/LowRangeはRightボタン/Leftボタンを表す
	AxisクラスがY軸（十字ボタンの上下）であれば、HighRange/LowRangeはUpボタン/Downボタンを表す
・使用例（概要）
	Axis axisX;
	PotMeter::State keyHint;
	auto btnState = axisX.GetState(keyHint);
	if (btnState == HwSwitch::State::Press)
	{
		if (keyHint == PotMeter::State::HighRange) { Print("右ボタンが押された"); }
		if (keyHint == PotMeter::State::LowRange) { Print("左ボタンが押された"); }
		//btnStateがPressの場合、(keyHint == MidRange)はあり得ない
	}
	if (btnState == HwSwitch::State::Free)
	{
		Print("右ボタン/左ボタンとも押されていない");
		//btnStateがFreeの場合、(keyHint == MidRange)以外はあり得ない
	}
*/
