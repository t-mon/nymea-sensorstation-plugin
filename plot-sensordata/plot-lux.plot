set term png small size 2000,1600
set output "lux.png"
set title 'Light intensity smoothing'

# X axsis
set xlabel 'Time'
set xdata time 
set timefmt "%s" 
set xtics format "%H:%M:%S"

# Y axsis
set ylabel 'Light intensity [lux]'

set autoscale
set grid

# labels
set label "Light intensity smoothing"

plot 'sensordata.log' using 1:8 with points title 'Measurment', \
     'sensordata.log' using 1:9 with lines title 'Smoothed'
