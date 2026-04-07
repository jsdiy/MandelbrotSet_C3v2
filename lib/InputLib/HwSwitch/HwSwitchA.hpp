//	ハードウェアスイッチ 非同期版
//	push_switch, toggle_switch, slide_switch, ...
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma	once

#include <Arduino.h>
#include <Ticker.h>
#include "HwSwitch.hpp"
#include "CallbackHandler.hpp"

//ハードウェアスイッチ 非同期版
class	HwSwitchA	: public HwSwitch
{
protected:
	static	constexpr	uint32_t	TickMSec = 50;
	//50ms --> 監視間隔は秒間20回	※DigitalInput::DebounceTimeMillisより長い方がよい

private:
	Ticker	tkPolling;
	CallbackHandler	onPress, onLongPress, onRelease;	//プッシュスイッチを想定した名前
	CallbackHandler	onRising, onFalling, onChange;	//attachInterrupt()のトリガーに対応するイベント
	void	EventTrigger(int8_t currentPinVal);

public:
	HwSwitchA() {}
	void	StartMonitoring(uint32_t tickMillis = TickMSec);	//監視開始	※UpdateState()をポーリングする
	void	StopMonitoring() { tkPolling.detach(); }	//監視停止
	int8_t	UpdateState();	//戻り値は現在のピン状態(HIGH/LOW)
	CallbackHandler&	OnPressCb() { return onPress; }			//スイッチオンの瞬間（プッシュスイッチを押した瞬間）
	CallbackHandler&	OnLongPressCb() { return onLongPress; }	//スイッチオン状態が長時間継続した瞬間（長押しが成立した瞬間）
	CallbackHandler&	OnReleaseCb() { return onRelease; }		//スイッチオフの瞬間（プッシュスイッチを放した瞬間）
	CallbackHandler&	OnRisingCb() { return onRising; }		//ピン状態がLOWからHIGHに変化した瞬間
	CallbackHandler&	OnFallingCb() { return onFalling; }		//ピン状態がHIGHからLOWに変化した瞬間
	CallbackHandler&	OnChangeCb() { return onChange; }		//ピン状態がHIGH/LOWの一方から他方へ変化した瞬間
};
