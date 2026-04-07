//	アナログ入力 非同期版
//	potentio_meter, slid_volune, stick/lever, ...
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include "PotMeter.hpp"
#include "CallbackHandler.hpp"

//入力監視・イベント通知
class	PotMeterA	: public PotMeter
{
protected:
	static	constexpr	uint32_t	TickMSec = 50;
	Ticker	tkPolling;
	CallbackHandler	onRiseToHigh, onFallFromHigh, onRiseFromLow, onFallToLow;
	void	EventTrigger(int16_t curentAdcVal);

public:
	PotMeterA() {}
	void	StartMonitoring(uint32_t tickMillis = TickMSec);	//監視開始	※UpdateState()をポーリングする
	void	StopMonitoring() { tkPolling.detach(); }	//監視停止
	int16_t	UpdateState();
	CallbackHandler&	OnRiseToHighCb() { return onRiseToHigh; }
	CallbackHandler&	OnFallFromHighCb() { return onFallFromHigh; }
	CallbackHandler&	OnRiseFromLowCb() { return onRiseFromLow; }
	CallbackHandler&	OnFallToLowCb() { return onFallToLow; }
};
