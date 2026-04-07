//	SPI-DMA制御
//	『昼夜逆転』工作室	@jsdiy	https://github.com/jsdiy

#include	<Arduino.h>
#include	"SpiDma.hpp"

//SpiDmaのインスタンス
//・ArduinoFrameworkがクラス'SPIClass'のインスタンス'SPI'を静的に生成していることに倣った。
//・ただし、インスタンス名をUpperCamelCaseにするとクラス名と同じになってしまうので（エラー）、lowerCamelCaseとした。
SpiDma	spiDma;

//SPIバスを初期化する
void	SpiDma::Initialize(spi_host_device_t hostId, gpio_num_t mosi, gpio_num_t miso, gpio_num_t sck, size_t bufSize)
{
	//SPIホストを設定する
	spiHostId = hostId;

	//SPIバスを設定する
	spi_bus_config_t spiBusConfig;
	InitializeSpiBusConfig(&spiBusConfig, mosi, miso, sck, bufSize);
	spi_bus_initialize(spiHostId, &spiBusConfig, SPI_DMA_CH_AUTO);

	//DMA送信バッファを確保する
	MAlloc(bufSize);
}

//SPIバスの設定（ユーザー設定値）
void	SpiDma::InitializeSpiBusConfig(spi_bus_config_t* spiBusConfig, gpio_num_t mosi, gpio_num_t miso, gpio_num_t sck, size_t bufSize)
{
	spiBusConfig->mosi_io_num = mosi;
	spiBusConfig->miso_io_num = miso;
	spiBusConfig->sclk_io_num = sck;
	spiBusConfig->quadwp_io_num = -1;
	spiBusConfig->quadhd_io_num = -1;
	spiBusConfig->data4_io_num = -1;
	spiBusConfig->data5_io_num = -1;
	spiBusConfig->data6_io_num = -1;
	spiBusConfig->data7_io_num = -1;
	spiBusConfig->max_transfer_sz = bufSize;
	spiBusConfig->flags = SPICOMMON_BUSFLAG_MASTER;
	spiBusConfig->intr_flags = ESP_INTR_FLAG_LEVEL1;
}

//SPIデバイスをバスに加える（3個まで可）
//引数	devConfig:	デバイスの設定
//		devHandle:	戻り値としてデバイスハンドル
bool	SpiDma::AddDeviceToBus(const spi_device_interface_config_t* devConfig, spi_device_handle_t* devHandle)
{
	esp_err_t retCode = spi_bus_add_device(spiHostId, devConfig, devHandle);
	return (retCode == ESP_OK);
}

//SPIデバイスをバスから外す
//・バスに接続したいデバイスが3個を越える場合はどれかを外す必要がある。
void	SpiDma::RemoveDeviceFromBus(spi_device_handle_t devHandle)
{
	spi_bus_remove_device(devHandle);
}

//DMA対応メモリを確保する
//・サイズ上限チェックは省略。
bool	SpiDma::MAlloc(size_t bufSize)
{
	if (dmaBuffer != nullptr) { heap_caps_free(dmaBuffer); dmaBuffer = nullptr; dmaBufferSize = 0; }
	dmaBuffer = (uint8_t*)heap_caps_malloc(bufSize, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
	bool isOK = (dmaBuffer != nullptr);
	if (isOK) { dmaBufferSize = bufSize; }
	Serial.printf("SpiDma dmaBuffer size: %d bytes. malloc: %s\n", bufSize, isOK ? "OK" : "NG");
	return isOK;
}

//データを送信する（転送完了までブロックする関数）
//・DMAではなくCPUによる転送。
void	SpiDma::Transmit(spi_device_handle_t devHandle, uint8_t data)
{
	trans.flags = SPI_TRANS_USE_TXDATA;	// *tx_bufferではなくtx_data[]を使うことのフラグ
	trans.length = ToBitLength(1);
	trans.tx_data[0] = data;

	spi_device_transmit(devHandle, &trans);
}

//データを送信する（転送完了までブロックする関数）
//・DMAではなくCPUによる転送。
void	SpiDma::Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2)
{
	trans.flags = SPI_TRANS_USE_TXDATA;	// *tx_bufferではなくtx_data[]を使うことのフラグ
	trans.length = ToBitLength(2);
	trans.tx_data[0] = data1;
	trans.tx_data[1] = data2;

	spi_device_transmit(devHandle, &trans);
}

//データを送信する（転送完了までブロックする関数）
//・DMAではなくCPUによる転送。
void	SpiDma::Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3)
{
	trans.flags = SPI_TRANS_USE_TXDATA;	// *tx_bufferではなくtx_data[]を使うことのフラグ
	trans.length = ToBitLength(3);
	trans.tx_data[0] = data1;
	trans.tx_data[1] = data2;
	trans.tx_data[2] = data3;

	spi_device_transmit(devHandle, &trans);
}

//データを送信する（転送完了までブロックする関数）
//・DMAではなくCPUによる転送。
void	SpiDma::Transmit(spi_device_handle_t devHandle, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4)
{
	trans.flags = SPI_TRANS_USE_TXDATA;	// *tx_bufferではなくtx_data[]を使うことのフラグ
	trans.length = ToBitLength(4);
	trans.tx_data[0] = data1;
	trans.tx_data[1] = data2;
	trans.tx_data[2] = data3;
	trans.tx_data[3] = data4;

	spi_device_transmit(devHandle, &trans);
}

//データを送信する（転送完了までブロックする関数）
//引数	datas:	DMA対応メモリであること
//		length <= dmaBufferSize
void	SpiDma::Transmit(spi_device_handle_t devHandle, const uint8_t* datas, size_t length)
{
	trans.flags = 0;	//tx_data[]ではなく *tx_bufferを使う場合はSPI_TRANS_USE_TXDATAフラグをリセットする
	trans.length = ToBitLength(length);
	trans.tx_buffer = datas;

	spi_device_transmit(devHandle, &trans);
}

//※基本的にこの関数の出番はない
//データを送信する（転送完了までブロックする関数）
//引数	datas:	DMA対応メモリであること
//		length > dmaBufferSize
void	SpiDma::TransmitOverSize(spi_device_handle_t devHandle, const uint8_t* datas, size_t length)
{
	trans.flags = 0;	//tx_data[]ではなく *tx_bufferを使う場合はSPI_TRANS_USE_TXDATAフラグをリセットする
	size_t idx = 0;
	while (0 < length)
	{
		size_t txLength = (dmaBufferSize < length) ? dmaBufferSize : length;
		trans.length = ToBitLength(txLength);
		trans.tx_buffer = &datas[idx];
		spi_device_transmit(devHandle, &trans);
		length -= txLength;
		idx += txLength;
	}
}

/*	非同期関係
//非同期でデータを送信する（転送完了までブロックしない関数）
//・ローカルのトランザクションデスクリプタを使用する。
bool	SpiDma::TransmitAsync(uint8_t* datas, size_t length)
{
	if (isTransmitAsync) { return false; }	//前回のTransmitAsync()がまだ完了していない（transが使用中）
	isTransmitAsync = true;
	TransmitAsync(datas, length, trans);
	return true;
}

//非同期でデータを送信する（転送完了までブロックしない関数）
//・ユーザーが用意したトランザクションデスクリプタを使用する。
void	SpiDma::TransmitAsync(uint8_t* datas, size_t length, spi_transaction_t* transaction)
{
	transaction->flags = 0;	//tx_data[]ではなく *tx_bufferを使う場合はフラグをリセットする
	transaction->length = ToBitLength(length);
	transaction->tx_buffer = datas;

	spi_device_queue_trans(spiDevice, transaction, portMAX_DELAY);
}

//TransmitAsync()の完了を待つ（ブロックする関数）
//戻り値:	実行完了したトランザクションデスクリプタ。またはnullptr
//・キューに入っているトランザクションの転送が完了するまで待つ。キューが空なら無限に待ち続ける。
//・キューの深さはspi_device_interface_config_t.queue_sizeで決めておく。
spi_transaction_t*	SpiDma::WaitForTransmitComplete(void)
{
	spi_transaction_t* finishedTrans = nullptr;
	spi_device_get_trans_result(spiDevice, &finishedTrans, portMAX_DELAY);

	//finishedTransがローカルのトランザクションデスクリプタだったら、戻り値はnullptrとする
	//ユーザーが用意したトランザクションデスクリプタだったら、それを戻り値とする
	return (finishedTrans == trans) ? nullptr : finishedTrans;
}
*/
