set term png small size 2000,1600
set output "temperature.png"
set title 'Temperature smoothing'

# X axsis
set xlabel 'Time'
set xdata time 
set timefmt "%s" 
set xtics format "%H:%M:%S"

# Y axsis
set ylabel 'Temperature [°C]'

set autoscale
set grid

# labels
set label "Temperature smoothing"

plot 'sensordata.log' using 1:2 with points title 'Measurment', \
     'sensordata.log' using 1:3 with lines title 'Smoothed'
