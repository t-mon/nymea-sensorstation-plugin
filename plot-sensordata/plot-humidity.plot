set term png small size 2000,1600
set output "humidity.png"
set title 'Humidity smoothing'

# X axsis
set xlabel 'Time'
set xdata time 
set timefmt "%s" 
set xtics format "%H:%M:%S"

# Y axsis
set ylabel 'Humidity [%]'

set autoscale
set grid

# labels
set label "Humidity smoothing"

plot 'sensordata.log' using 1:4 with points title 'Measurment', \
     'sensordata.log' using 1:5 with lines title 'Smoothed'
