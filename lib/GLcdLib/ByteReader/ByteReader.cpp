//	byte配列からバッファへ順次読み込むクラス
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2026/02	初版

#include <Arduino.h>
#include "ByteReader.hpp"

//byte配列からDMAバッファへ、順次コピーする
/*
bytes[][][][][][][][][][][][][][][][][][]...
|srcLength		:				:		|...
|bufCopyLen		|bufCopyLen		|bCpLn	|...	最後はそれまでより短い可能性がある
*/
void	ByteReader::SetLinear(uint8_t* buffer, size_t bufSize, const uint8_t* datas, size_t length)
{
	linear.buf = buffer;
	linear.bufSize = bufSize;
	linear.datas = datas;
	linear.srcLength = length;
	linear.idx = 0;

	Next = [this]() { return this->linear.Next(); };
	//↑ '=[オブジェクト](引数){処理}'はC++のラムダ式。std::function<>にメンバー関数を代入する際のモダン（C++11以降）な書き方
}

size_t	ByteReader::Linear::Next()
{
	if (idx == srcLength) { return 0; }

	size_t bufCopyLength = std::min(srcLength - idx, bufSize);
	memcpy(buf, &datas[idx], bufCopyLength);
	idx += bufCopyLength;

	return bufCopyLength;
}

//byte配列からDMAバッファへ、一定間隔で一定量をコピーする動作を指定回数繰り返す
/*
bytes[][][][][][][][][][][][][][][][][][][][][][]...
|stepLength		|stepLength		|stepLength		|...	一定間隔
|copyLength|xxxx|copyLength|xxxx|copyLength|xxxx|...	一定量（x部分はコピーされない）
repeat:1		repeat:2		repeat:3		...
*/
void	ByteReader::SetStrided(uint8_t* buffer, size_t bufSize, const uint8_t* datas, size_t stepLength, size_t copyLength, int16_t repeat)
{
	//「一定量」のコピーサイズがバッファサイズより大きかったらこの関数は実行できない
	//・ただし、グラフィックLCDへ描画する画像データの場合、サイズ制限は考慮するまでもない。
	//	仮にRGB565の画像データでバッファ16KBとすると、画像の幅が8192画素(=16KB/2byte)を超えない限り実行できる。
	if (bufSize < copyLength) { return; }

	strided.buf = buffer;
	strided.bufSize = bufSize;
	strided.datas = datas;
	strided.stepLength = stepLength;
	strided.copyLength = copyLength;
	strided.repeatMax = repeat;
	strided.repeatCount = 0;

	Next = [this]() { return this->strided.Next(); };
	//↑ '=[オブジェクト](引数){処理}'はC++のラムダ式。std::function<>にメンバー関数を代入する際のモダン（C++11以降）な書き方
}

size_t	ByteReader::Strided::Next()
{
	if (repeatCount == repeatMax) { return 0; }

	//1回でbufSize上限まで、datasからcopyBytes単位でコピーする
	size_t idx = 0;
	while ((idx + copyLength < bufSize) && (repeatCount < repeatMax))
	{
		memcpy(&buf[idx], &datas[stepLength * repeatCount], copyLength);
		repeatCount++;
		idx += copyLength;
	}

	return idx;
}
