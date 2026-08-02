#include "pxt.h"
#include <cstdint>

namespace ds18b20 {

    // 1ビット書き込み（割り込みを無効化してタイミングを死守）
    void writeBit(MicroBitPin* pin, int bit) {
        __disable_irq(); // 割り込み禁止
        pin->setDigitalValue(0);
        target_wait_us(2);
        if (bit) {
            pin->setDigitalValue(1);
        }
        target_wait_us(60);
        pin->setDigitalValue(1);
        target_wait_us(2);
        __enable_irq(); // 割り込み許可
    }

    // 1ビット読み込み
    int readBit(MicroBitPin* pin) {
        __disable_irq();
        pin->setDigitalValue(0);
        target_wait_us(2);
        pin->setDigitalValue(1);
        target_wait_us(8);
        int b = pin->getDigitalValue();
        target_wait_us(50);
        __enable_irq();
        return b;
    }

    // 1バイト書き込み
    void writeByte(MicroBitPin* pin, int byte) {
        for (int i = 0; i < 8; i++) {
            writeBit(pin, byte & 1);
            byte >>= 1;
        }
    }

    // 1バイト読み込み
    int readByte(MicroBitPin* pin) {
        int byte = 0;
        for (int i = 0; i < 8; i++) {
            byte |= (readBit(pin) << i);
        }
        return byte;
    }

    //%
    int readTemp(int pinId) {
        // ピンの取得（micro:bit v1/v2 両対応）
        MicroBitPin* pin = pxt::getPin(pinId);
        if (!pin) return -99900;

        // 1. 初期化とプレゼンスパルスの確認
        pin->setDigitalValue(0);
        target_wait_us(480);
        pin->setDigitalValue(1);
        target_wait_us(70);
        int presence = pin->getDigitalValue();
        target_wait_us(410);

        if (presence != 0) return -8500; // センサー未接続エラー

        // 2. ROMスキップ (0xCC) ＆ 温度変換開始 (0x44)
        writeByte(pin, 0xCC);
        writeByte(pin, 0x44);

        // 3. 変換待ち (約750ms必要。この間は他のタスクを動かすためfiber_sleepを使用)
        fiber_sleep(750);

        // 4. 再び初期化
        pin->setDigitalValue(0);
        target_wait_us(480);
        pin->setDigitalValue(1);
        target_wait_us(70);
        presence = pin->getDigitalValue();
        target_wait_us(410);

        // 5. ROMスキップ (0xCC) ＆ スクラッチパッド読み出し (0xBE)
        writeByte(pin, 0xCC);
        writeByte(pin, 0xBE);

        // 6. 2バイト読み込み（下位バイト, 上位バイト）
        int lsb = readByte(pin);
        int msb = readByte(pin);

        // 7. 温度計算 (12bit分解能の場合、0.0625℃単位)
        int16_t raw = (msb << 8) | lsb;
        
        // TypeScript側で少数として扱うため、100倍した整数を返す (例: 25.53℃ -> 2553)
        return (raw * 100) / 16;
    }
}
