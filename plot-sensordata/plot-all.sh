#/bin/bash

scp root@10.10.10.120:/tmp/sensordata.log .

gnuplot plot-temperature.plot
gnuplot plot-lux.plot
gnuplot plot-pressure.plot
gnuplot plot-humidity.plot
gnuplot plot-ppm.plot
gnuplot plot-temperature.plot
