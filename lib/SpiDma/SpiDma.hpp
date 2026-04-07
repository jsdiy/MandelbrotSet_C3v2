//	SPI-DMA制御
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy
//	2024/06 - 2025/09	初版, 小規模改良
//	2025/10	namespace_SpiPinConfigを追加, それに伴いInitialize()を更新
//	2026/01	dmaBufferをこのクラスで持つよう変更, それをどこからでもGetBuffer()で取得できるよう変更

#pragma	once

#include <Arduino.h>
#include "driver/spi_master.h"

class	SpiDma
{
private:	//定数
#if	CONFIG_IDF_TARGET_ESP32
	static	constexpr	spi_host_device_t	HostID = VSPI_HOST;
	static	constexpr	gpio_num_t	PinMOSI = GPIO_NUM_23, PinMISO = GPIO_NUM_19, PinSCK = GPIO_NUM_18, PinSS = GPIO_NUM_5;
	static	constexpr	size_t DefaultBufferSize = 32UL * 1024;	//16KB-24KB:速度とメモリ量のバランス, 32KB-48KB:高スループット狙い
#endif
#if	CONFIG_IDF_TARGET_ESP32C3
	static	constexpr	spi_host_device_t	HostID = SPI2_HOST;
	static	constexpr	gpio_num_t	PinMOSI = GPIO_NUM_7, PinMISO = GPIO_NUM_2, PinSCK = GPIO_NUM_6, PinSS = GPIO_NUM_10;
	static	constexpr	size_t DefaultBufferSize = 24UL * 1024;	//16KB:速度とメモリ量のバランス, 24KB-28KB:高スループット狙い
#endif
	/*	spi_bus_config_t.max_transfer_sz に指定する値について
	SPI-DMAのデスクリプタ1つで4092byteまで転送可能
	max_transfer_sz=0とした場合は4092byteを指定したことになる
	max_transfer_szは、ESP32では64KBまで指定可能, ESP32-C3では262143bit(=32KB-1bit)まで指定可能
	*/

public:	//DMA対応バッファ関連
	uint8_t*	GetBuffer() { return dmaBuffer; }
	size_t	BufferSize() { return dmaBufferSize; }

private:
	spi_host_device_t	spiHostId;
	spi_transaction_t	trans;
	SemaphoreHandle_t	spiMutex;	//SPIバスの排他利用制御用
	uint8_t*	dmaBuffer;
	size_t	dmaBufferSize;
	size_t	ToBitLength(size_t byteLength) const { return byteLength * 8; }
	void	InitializeSpiBusConfig(spi_bus_config_t* spiBusConfig, gpio_num_t mosi, gpio_num_t miso, gpio_num_t sck, size_t bufSize);
	bool	MAlloc(size_t bufSize);

public:
	SpiDma() : dmaBuffer(nullptr), dmaBufferSize(0) { spiMutex = xSemaphoreCreateMutex(); }
	void	Initialize(spi_host_device_t hostId = HostID,
				gpio_num_t mosi = PinMOSI, gpio_num_t miso = PinMISO, gpio_num_t sck = PinSCK, size_t bufSize = DefaultBufferSize);
	bool	AddDeviceToBus(const spi_device_interface_config_t* devConfig, spi_device_handle_t* devHandle);
	void	RemoveDeviceFromBus(spi_device_handle_t devHandle);
	void	TakeBusControll() { xSemaphoreTake(spiMutex, pdMS_TO_TICKS(1000)); }	//有限時間（デッドロック低減）
	void	GiveBusControll() { xSemaphoreGive(spiMutex); }
	void	Transmit(spi_device_handle_t devHandle, uint8_t data);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3);
	void	Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);
	void	Transmit(spi_device_handle_t devHandle, const uint8_t* datas, size_t length);
	void	TransmitOverSize(spi_device_handle_t devHandle, const uint8_t* datas, size_t length);	//基本的にこの関数の出番はない
};

extern	SpiDma	spiDma;
