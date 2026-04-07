//	ジョイスティック
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "Joystick.hpp"

//レバーとボタンを初期化する
void	Joystick::Initialize(gpio_num_t axisX, gpio_num_t axisY, bool invertX, bool invertY, int8_t newtralRangeH, int8_t newtralRangeL,
			gpio_num_t swP, gpio_num_t swA, gpio_num_t swB, gpio_num_t swC)
{
	StickConfig(axisX, axisY, invertX, invertY, newtralRangeH, newtralRangeL);
	ButtonConfig(swP, swA, swB, swC);
}

//レバーを初期化／再設定する
//invertX:	左右方向を反転させる
//invertY:	上下方向を反転させる
//newtralRange:	レバーの感度(1～9)　中立から最大傾倒までのうち、何割までを中立の範囲とするか
void	Joystick::StickConfig(gpio_num_t axisX, gpio_num_t axisY, bool invertX, bool invertY, int8_t newtralRangeH, int8_t newtralRangeL)
{
	StopKeyMonitoring();

	this->axisX.Initialize(axisX);
	this->axisY.Initialize(axisY);

	this->axisX.SetInverted(invertX ? this->axisX.AdcValueMax : 0);
	this->axisY.SetInverted(invertY ? this->axisY.AdcValueMax : 0);

	StickSensitivity(newtralRangeH, newtralRangeL);

	StartKeyMonitoring();
}

//レバーの感度を設定する
//newtralRange:	1から9
//・中立から最大傾倒までのうち、何割までを中立の範囲とするか。
//	H: ADC値の高い側（右、上）, L: ADC値の低い側（左、下）
void	Joystick::StickSensitivity(int8_t newtralRangeH, int8_t newtralRangeL)
{
	newtralRangeH = std::max(newtralRangeH, (int8_t)1);
	newtralRangeH = std::min(newtralRangeH, (int8_t)9);
	newtralRangeL = std::max(newtralRangeL, (int8_t)1);
	newtralRangeL = std::min(newtralRangeL, (int8_t)9);

	int32_t center = axisX.Read();	//理論上は0とmaxとの中間値
	int16_t range = (int16_t)(center * newtralRangeH / 10);
	axisX.SetRisingThreshold(center + range);
	range = (int16_t)(center * newtralRangeL / 10);
	axisX.SetFallingThreshold(center - range);

	center = axisY.Read();	//理論上は0とmaxとの中間値
	range = (int16_t)(center * newtralRangeH / 10);
	axisY.SetRisingThreshold(center + range);
	range = (int16_t)(center * newtralRangeL / 10);
	axisY.SetFallingThreshold(center - range);
}

//ボタンを初期化／再設定する
void	Joystick::ButtonConfig(gpio_num_t swP, gpio_num_t swA, gpio_num_t swB, gpio_num_t swC)
{
	StopKeyMonitoring();

	//pinでbtnを初期化し、keyをキーにしてbtnを辞書登録する
	auto ButtonEmplace = [this](gpio_num_t pin, JButton& btn, KeyCode key)
	{
		if (pin == GPIO_NUM_NC) { this->buttons.emplace(key, nullptr); }
		else { btn.Initialize(pin); this->buttons.emplace(key, &btn); }
	};

	buttons.clear();
	ButtonEmplace(swP, this->btnP, KeyCode::P);
	ButtonEmplace(swA, this->btnA, KeyCode::A);
	ButtonEmplace(swB, this->btnB, KeyCode::B);
	ButtonEmplace(swC, this->btnC, KeyCode::C);

	StartKeyMonitoring();
}

//ボタンを取得する
//・存在しないボタンキーの場合（ORされているなど）、nullptrを返す。
//・有効なボタンキーでもボタン自体が無効の場合、結果的にnullptrが返る。
//	例）ピンが設定されていないbtnCの実体はなくnullptr。→ButtonConfig()
Joystick::JButton*	Joystick::GetButton(KeyCode key)
{
	auto iter = buttons.find(key);
	return (iter == buttons.end()) ? nullptr : iter->second;
}

//キーを何ミリ秒押したら長押し成立とするか
void	Joystick::SetLongHoldThresholdTime(uint32_t millis)
{
	axisX.SetLongHoldThresholdTime(millis);
	axisY.SetLongHoldThresholdTime(millis);
	for (auto& pair : buttons) { if (pair.second) { pair.second->SetLongHoldThresholdTime(millis); } }
}

//キー入力状態の監視を開始する
void	Joystick::StartKeyMonitoring()
{
	tkPolling.attach_ms(pollingTime, +[](Joystick* me) { me->UpdateStateAndSetBits(); }, this);
}

//キー入力状態の監視を停止する（キーアサイン変更前の呼び出しを想定）
//・イベントを拾わないようにする。キー操作変更を「なし」にする
//・全てのコールバックを登録解除（削除）する。
//・全てのキー操作状態／イベントフラグをクリアする。
void	Joystick::StopKeyMonitoring()
{
	tkPolling.detach();	//ポーリングを停止する
	KeyEventTrigger(false);	//イベントによるコールバック発火を停止する
	isUpdatedKeyState = false;	//キー操作の変更有無を「なし」にする

	//コールバックを削除する
	pressCbs.RemoveAll();
	longPressCbs.RemoveAll();
	releaseCbs.RemoveAll();

	//キー操作のフラグ類をクリアする
	keyHoldingBits = 0;
	onPressBits = 0;
	onLongPressBits = 0;
	onReleaseBits = 0;
	adcValX = adcValY = 0;
}

//キー操作状態が更新されたか問い合わせる
//戻り値：	true: キー状態が更新された,	false: キー状態に変化はなかった
//・Tickerが各ステータスフラグを更新したタイミングでtrueが返る。
bool	Joystick::CheckKeyState()
{
	if (isUpdatedKeyState)
	{
		isUpdatedKeyState = false;
		return true;
	}
	else
	{
		return false;
	}
}

//キー入力状態を更新し、ビットフラグをセットする
//・Tickerから呼ばれる。この関数はアプリ側には公開しない。
void	Joystick::UpdateStateAndSetBits()
{
//auto startMs = millis();

	//キー入力状態を更新する
	adcValX = axisX.UpdateState();
	adcValY = axisY.UpdateState();
	for (auto& pair : buttons) { if (pair.second) { pair.second->UpdateState(); } }

	//キーのオン状態をスキャンする
	keyHoldingBits = 0;
	if (axisX.IsHighRange()) { keyHoldingBits |= KeyCode::Right; }
	if (axisX.IsLowRange()) { keyHoldingBits |= KeyCode::Left; }
	if (axisY.IsHighRange()) { keyHoldingBits |= KeyCode::Up; }
	if (axisY.IsLowRange()) { keyHoldingBits |= KeyCode::Down; }
	for (const auto& pair : buttons)
	{
		if (pair.second && pair.second->IsSwOn()) { keyHoldingBits |= pair.first; }
	}

	using	AState = JStick::State;
	using	DState = JButton::State;

	auto ScanButtonsEvent = [this](DState eventCode, std::atomic<uint16_t>& bitFlags)
	{
		for (const auto& pair : this->buttons)
		{
			if (pair.second && (pair.second->GetState() == eventCode)) { bitFlags |= pair.first; }
		}
	};

	auto StickEvent = [](DState eventCode, std::atomic<uint16_t>& bitFlags, JStick& axis, KeyCode highKey, KeyCode lowKey)
	{
		AState keyHint;
		if (axis.GetState(keyHint) != eventCode) { return; }
		if (keyHint == AState::HighRange) { bitFlags |= highKey; }
		if (keyHint == AState::LowRange) { bitFlags |= lowKey; }
	};

	auto ScanSticksEvent = [&StickEvent, this](DState eventCode, std::atomic<uint16_t>& bitFlags)
	{
		StickEvent(eventCode, bitFlags, this->axisX, KeyCode::Right, KeyCode::Left);
		StickEvent(eventCode, bitFlags, this->axisY, KeyCode::Up, KeyCode::Down);
	};

	auto ScanKeyEvent = [&ScanButtonsEvent, &ScanSticksEvent](DState eventCode, std::atomic<uint16_t>& bitFlags)
	{
		ScanButtonsEvent(eventCode, bitFlags);
		ScanSticksEvent(eventCode, bitFlags);
	};

	//キー押下イベントの発生をスキャンする
	onPressBits = 0;
	ScanKeyEvent(DState::Press, onPressBits);

	//キー長押しイベントの発生をスキャンする
	onLongPressBits = 0;
	ScanKeyEvent(DState::LongPress, onLongPressBits);

	//キーを放したイベントの発生をスキャンする
	onReleaseBits = 0;
	ScanKeyEvent(DState::Release, onReleaseBits);

	//コールバックを実行する
	if (keyEventTriggerEnabled)
	{
		pressCbs.Fire(onPressBits);
		longPressCbs.Fire(onLongPressBits);
		releaseCbs.Fire(onReleaseBits);
	}

	//キー状態を更新したことのフラグを立てる
	isUpdatedKeyState = true;

//Serial.printf("UpdateStateAndSetBits(): %d ms\n", millis() - startMs);	//1ms以下だった
}

bool	Joystick::OnKeyPress(KeyCode keys) { return (onPressBits & keys) == keys; }
bool	Joystick::OnKeyLongPress(KeyCode keys) { return (onLongPressBits & keys) == keys; }
bool	Joystick::OnKeyRelease(KeyCode keys) { return (onReleaseBits & keys) == keys; }
bool	Joystick::IsKeyHolding(KeyCode keys) { return (keyHoldingBits & keys) == keys; }
bool	Joystick::IsKeyFree(KeyCode keys) { return !IsKeyHolding(keys); }
//※デバッグ時、ビットフラグを表示するには↓このような方法がある
//#include <bitset>
//Serial.printf("onPressBits=%s, keys=%s\n", std::bitset<16>(onPressBits).to_string().c_str(), std::bitset<16>((uint16_t)keys).to_string().c_str());
