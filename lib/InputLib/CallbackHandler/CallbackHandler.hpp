//	コールバック関数を登録・呼び出すクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/03	初版

#pragma	once

#include <Arduino.h>
#include <Ticker.h>
#include <functional>

class	CallbackHandler
{
private:
	Ticker	ticker;	//Invokeで利用する
	std::function<void()>	FnCallback = nullptr;
	std::function<void(uint32_t)>	FnCallbackWithArg = nullptr;
	uint32_t	arg;	//Invoke(n)の呼び出しごとに更新されるが、呼び出し間隔がTicker.once_ms()以上なら問題ない

public:
	CallbackHandler() {}
	void	Add(std::function<void()> callback);
	void	Add(std::function<void(uint32_t)> callback);
	void	Invoke(uint32_t n = 0);
	void	Remove();
};
