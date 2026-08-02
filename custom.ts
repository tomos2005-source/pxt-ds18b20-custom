/**
 * DS18B20 温度センサー用の拡張機能ブロック
 */
//% weight=100 color=#E3008C icon="\uf2c9" block="DS18B20"
namespace DS18B20 {

    /**
     * C++関数のシム（シミュレーター用ダミー関数）
     * 実際のmicro:bitではC++の ds18b20::readTemp が呼ばれます。
     */
    //% shim=ds18b20::readTemp
    function readTemp(pin: DigitalPin): number {
        // ブラウザのシミュレーター上では常に 25.5℃ (2550) を返す
        return 2550;
    }

    /**
     * DS18B20から温度（℃）を読み込みます
     * @param pin 接続先のピン, eg: DigitalPin.P0
     */
    //% blockId="ds18b20_get_temperature" block="DS18B20(ピン %pin) の温度(℃)"
    //% weight=100
    export function getTemperature(pin: DigitalPin): number {
        // C++側で処理した温度データ（100倍された整数）を取得
        let temp100 = readTemp(pin);

        // センサーが見つからない場合のエラーコード処理
        if (temp100 == -8500 || temp100 == -99900) {
            return -999; // エラー時は-999を返す
        }

        // 100で割って実際の温度（℃）に戻す
        return temp100 / 100.0;
    }
}