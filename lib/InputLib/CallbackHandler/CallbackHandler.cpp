//	コールバック関数を登録・呼び出すクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include <Arduino.h>
#include "CallbackHandler.hpp"

/*	void callback() を登録する
--> Add(\[this]\(){ this->callback(); })
※callback()が他のクラス内にあるなら'this'ではなく、そのクラスのインスタンスとする
例) FooClass* obj; なら'this' は 'obj'
*/
void	CallbackHandler::Add(std::function<void()> callback)
{
	Remove();
	FnCallback = callback;
}

/*	void callback(uint32_t) を登録する
--> Add(\[this]\(uint32_t n){ this->callback(n); })
※callback()が他のクラス内にあるなら'this'ではなく、そのクラスのインスタンスとする
例) FooClass* obj; なら'this' は 'obj'
*/
void	CallbackHandler::Add(std::function<void(uint32_t)> callback)
{
	Remove();
	FnCallbackWithArg = callback;
}

void	CallbackHandler::Invoke(uint32_t n)
{
	if (FnCallbackWithArg != nullptr)
	{
		arg = n;
		ticker.once_ms(1, +[](CallbackHandler* me){ me->FnCallbackWithArg(me->arg); }, this);
	}
	else if (FnCallback != nullptr)
	{
		ticker.once_ms(1, +[](CallbackHandler* me){ me->FnCallback(); }, this);
	}
}

void	CallbackHandler::Remove()
{
	FnCallback = nullptr;
	FnCallbackWithArg = nullptr;
	arg = 0;
}
