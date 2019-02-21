set term png small size 2000,1600
set output "pressure.png"
set title 'Pressure smoothing'

# X axsis
set xlabel 'Time'
set xdata time 
set timefmt "%s" 
set xtics format "%H:%M:%S"

# Y axsis
set ylabel 'Pressure [hPa]'

set autoscale
set grid

# labels
set label "Pressure smoothing"

plot 'sensordata.log' using 1:6 with points title 'Measurment', \
     'sensordata.log' using 1:7 with lines title 'Smoothed'
