set term png small size 2000,1600
set output "ppm.png"
set title 'Air quality smoothing'

# X axsis
set xlabel 'Time'
set xdata time 
set timefmt "%s" 
set xtics format "%H:%M:%S"

# Y axsis
set ylabel 'Air quality [ppm]'

set autoscale
set grid

# labels
set label "Air quality smoothing"

plot 'sensordata.log' using 1:10 with points title 'Measurment', \
     'sensordata.log' using 1:11 with lines title 'Smoothed'
