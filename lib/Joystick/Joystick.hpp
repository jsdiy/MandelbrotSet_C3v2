//	ジョイスティック
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include <unordered_map>
#include <vector>
#include <atomic>
#include "PotMeter.hpp"
#include "HwSwitch.hpp"
#include "CallbackHandler.hpp"
#include "Axis.hpp"

class	Joystick
{
public:
	//スティック／ボタンのキーコード
	//・スティックの各方向と、各ボタンをキーと呼ぶこととする。
	//・同時押しを考慮してビットフラグで定義する。
	enum	class	KeyCode	: uint16_t
	{
		Up		= 0x0001,	//bit:0
		Down	= 0x0002,	//bit:1
		Left	= 0x0004,	//bit:2
		Right	= 0x0008,	//bit:3
		P		= 0x0010,	//bit:4
		A		= 0x0020,	//bit:5
		B		= 0x0040,	//bit:6
		C		= 0x0080,	//bit:7
	};

	//コールバック登録
	class	CallbackContainer
	{
	private:
		using	KeyFuncPair = std::pair<KeyCode, CallbackHandler*>;
		std::vector<KeyFuncPair>	vec;

	public:
		//ArduinoFrameworkによる別名定義'OnReceiveCb'が煩わしいので別名を付け直す
		using	KeyEventCb = std::function<void()>;

		/*	キー操作イベント発生時のコールバックを登録する
		・KeyCodeはORも可。例）A+Bボタン同時押しをトリガーとする場合、KeyCode::A|KeyCode::B
		・キー操作が複数のイベント発生条件に当てはまる場合、コールバックは登録順優先で1つだけ実行される。
			例）(1)Aボタン押下 (2)A+Bボタン同時押下 のコールバックがこの順に登録されているとして、
				A+Bボタンが同時に押された場合、(1)のコールバックが実行され、(2)は実行されない。
			例）(2)(1)の順に登録されているとして、
				A+Bボタンが同時に押された場合、(2)のコールバックが実行され、(1)は実行されない。
		*/
		void	Add(KeyCode keys, KeyEventCb func)
		{
			CallbackHandler* cbHandler = new CallbackHandler();
			cbHandler->Add(func);
			vec.emplace_back(keys, cbHandler);
		}

		//コールバックを全て削除する
		void	RemoveAll()
		{
			for (auto& pair : vec) { pair.second->Remove(); delete pair.second; }
			vec.clear();
		}

		//コールバックを実行する
		void	Fire(std::atomic<uint16_t>& keyBits)
		{
			for (const auto& pair : vec)
			{
				uint16_t keys = static_cast<uint16_t>(pair.first);
				if ((keyBits & keys) == keys) { pair.second->Invoke(keys); }
			}
		}

		//デストラクタ
		~CallbackContainer() { RemoveAll(); }
	};

public:
	using	KeyState	= HwSwitch::State;
	static	constexpr	uint32_t	KeyCombinationTime = 50;	//キーの同時押し許容時間(ms)　※デバウンス時間より長いこと

private:
	using	JStick	= Axis;	//PotMeter;
	using	JButton	= HwSwitch;
	JStick	axisX, axisY;
	JButton	btnP, btnA, btnB, btnC;
	std::unordered_map<KeyCode, JButton*>	buttons;
	JButton*	GetButton(KeyCode key);
	uint32_t	keyCombinationTime = KeyCombinationTime;
	void	StickSensitivity(int8_t newtralRangeH, int8_t newtralRangeL);

	CallbackContainer	pressCbs;
	CallbackContainer	longPressCbs;
	CallbackContainer	releaseCbs;

	std::atomic<uint16_t>	keyHoldingBits{0};	//現在のキーオン／オフ状態
	std::atomic<uint16_t>	onPressBits{0};		//OnPressイベント発生有無
	std::atomic<uint16_t>	onLongPressBits{0};	//OnLongPressイベント発生有無
	std::atomic<uint16_t>	onReleaseBits{0};	//OnReleaseイベント発生有無
	std::atomic<int16_t>	adcValX{0}, adcValY{0};

	Ticker	tkPolling;
	uint32_t	pollingTime = keyCombinationTime;
	uint32_t	prevUpdateTime = 0;
	volatile	bool	isUpdatedKeyState = false;
	bool	keyEventTriggerEnabled = false;
	void	UpdateStateAndSetBits();
	void	StartKeyMonitoring();	//キー入力監視を開始する
	void	StopKeyMonitoring();	//キー入力監視を停止する　※Config()時に一旦停止させる必要がある

	//アプリ側でこれらを使う場面はなさそう。とりあえずprivateとしておく
	void	SetKeyCombinationTime(uint32_t millis) { pollingTime = keyCombinationTime = millis; }	//複数キーを何ミリ秒以内に押したら同時押しとするか
	void	StickConfig(gpio_num_t axisX, gpio_num_t axisY, bool invertX, bool invertY, int8_t newtralRangeH, int8_t newtralRangeL);
	void	ButtonConfig(gpio_num_t swP, gpio_num_t swA, gpio_num_t swB, gpio_num_t swC);

public:
	Joystick() {}
	void	Initialize(gpio_num_t axisX, gpio_num_t axisY, bool invertX, bool invertY, int8_t newtralRangeH, int8_t newtralRangeL,
				gpio_num_t swP, gpio_num_t swA, gpio_num_t swB, gpio_num_t swC);
	void	SetLongHoldThresholdTime(uint32_t millis);

	bool	CheckKeyState();
	bool	OnKeyPress(KeyCode keys);		//キーが押された瞬間である
	bool	OnKeyLongPress(KeyCode keys);	//キーの長押しが成立した瞬間である
	bool	OnKeyRelease(KeyCode keys);		//キーが放された瞬間である
	bool	IsKeyHolding(KeyCode keys);		//キーが押されている（長押しかどうかに関係なく。また、押した瞬間も含む）
	bool	IsKeyFree(KeyCode keys);		//キーが放されている（放した瞬間も含む）
	int16_t	AdcAxisX() const { return adcValX; }	//現在のX軸のADC値
	int16_t	AdcAxisY() const { return adcValY; }	//現在のY軸のADC値
	
	void	KeyEventTrigger(bool enable) { keyEventTriggerEnabled = enable; }	//true:イベント発生時、コールバックを実行する, false:実行しない
	CallbackContainer&	OnPressCb() { return pressCbs; }	//「キー押下」イベント発生時のコールバック関数を登録する
	CallbackContainer&	OnLongPressCb() { return longPressCbs; }	//「キー長押し」イベント発生時のコールバック関数を登録する
	CallbackContainer&	OnReleaseCb() { return releaseCbs; }	//「キー解放」イベント発生時のコールバック関数を登録する
};

inline Joystick::KeyCode	operator | (Joystick::KeyCode lhs, Joystick::KeyCode rhs)
{
	return static_cast<Joystick::KeyCode>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

inline	std::atomic<uint16_t>&	operator |= (std::atomic<uint16_t>& lhs, Joystick::KeyCode rhs)
{
	lhs |= static_cast<uint16_t>(rhs);
	return lhs;
}

inline	uint16_t	operator & (std::atomic<uint16_t>& lhs, Joystick::KeyCode rhs)
{
	return static_cast<uint16_t>(lhs & static_cast<uint16_t>(rhs));
}

inline	bool	operator == (uint16_t lhs, Joystick::KeyCode rhs)
{
	return (lhs == static_cast<uint16_t>(rhs));
}
