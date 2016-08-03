# powermon

Software for reading the ADC on the Intel Galileo Gen 2 board and for calculating electric power in a three phase system. The hardware required is more or less the same as [EmonTx shield](https://openenergymonitor.org/emon/emontxshield) without the RF transceiver.

The calculated values are sent to [InfluxDB](https://influxdata.com/time-series-platform/influxdb/).

Requires [libiio](https://github.com/analogdevicesinc/libiio) and [libcurl](https://curl.haxx.se/libcurl/).
