%=================================================================
% Creates a 2 real time graph from 4 bytes received from serial
% The Arduino/Energia code used is named "analog_2_values_sender.ino"
% Change the COM name to the one you want in the function serial()
%=================================================================

function nada = teste()

% delete all serial ports from memory 
% important, if you the code is stopped without closing and deleting the
% used COM, you need to do this to open it again
delete(instrfindall);


% Init and open the serial port
s = serial('COM21', 'baudrate', 9600);
fopen(s);


%close all open figures
close ALL
figure


%set the maximum for the values 
temp_max = 40;
temp_min = -10;
moist_max = 4095;
moist_min = 0;

air_temp_max = 40;
air_temp_min = -10;
air_humidity_max = 100;
air_humidity_min = 0;

lux_max = 1000;
lux_min = 0;

points = 500; %number of points on the graph at all times
data_period = 96; %data period in milliseconds 
%x will be the time axis. The time between points is defined by
%"data_period"
x = linspace(-points*data_period,0, points);

% draw a area graph with parameter x and y. Save the return value so we can
% edit the graph later. You can use "plot" instead of "area", I just liked
% the aspect better.
subplot(2,3,1);
y_soil_temp = linspace(0,0,points);
lh_soil_temp = area(x,y_soil_temp, 'FaceColor', [.5 .5 .5]);
str = sprintf('Temperature, %d to %dK\%', temp_min, temp_max),
title(str);
axis([-points*data_period, 0, temp_min, temp_max]);
%line([-points*data_period, 0] , [ 2048 2048]);

subplot(2,3,2);
y_soil_moisture = linspace(0,0,points);
lh_soil_moisture = area(x,y_soil_moisture, 'FaceColor', [.5 .5 .5]);
str = sprintf('Moisture, %d to %d (readings\%', moist_min, moist_max),
title(str);
axis([-points*data_period, 0, moist_min, moist_max]);
%line([-points*data_period, 0] , [ 2048 2048]);


subplot(2,3,3);
y_air_temp = linspace(0,0,points);
lh_air_temp = area(x,y_air_temp, 'FaceColor', [.5 .5 .5]);
str = sprintf('Air Temperature, %d to %d (readings\%', air_temp_min, air_temp_max),
title(str);
axis([-points*data_period, 0, air_temp_min, air_temp_max]);

subplot(2,3,4);
y_air_humidity = linspace(0,0,points);
lh_air_humidity = area(x,y_air_humidity, 'FaceColor', [.5 .5 .5]);
str = sprintf('Relative Air Humidity, %d to %d (readings\%', air_humidity_min, air_humidity_max),
title(str);
axis([-points*data_period, 0, air_humidity_min, air_humidity_max]);


subplot(2,3,5);
y_lux = linspace(0,0,points);
lh_lux = area(x,y_lux, 'FaceColor', [.5 .5 .5]);
str = sprintf('Lux, %d to %d (readings\%', lux_min, lux_max),
title(str);
axis([-points*data_period, 0, lux_min, lux_max]);

shg; %brings the figure to the front of all other windows

key = get(gcf,'CurrentKey'); %get the key currently pressed
%this "while" will stop if you press the "s" key
while ( strcmp(key, 's') == 0 ) 
    key = get(gcf,'CurrentKey'); %get the key currently pressed
    
    command = 0; size = 0;
    while command ~= 18 || size < 2*5
         [command, size, values] = get_1_packet(s);
    end
    
    
    temperature = (bitsll(values(2), 8) +  values(1))/100
    y_soil_temp = shift_in_value(y_soil_temp, points, temperature);

  
    moisture = (bitsll(values(4), 8) +  values(3))
    y_soil_moisture = shift_in_value(y_soil_moisture, points, moisture);

    air_temp = (bitsll(values(6), 8) +  values(5))/100
    y_air_temp = shift_in_value(y_air_temp, points, air_temp);
  
    air_humidity = (bitsll(values(8), 8) +  values(7))/100
    y_air_humidity = shift_in_value(y_air_humidity, points, air_humidity);

    lux = (bitsll(values(10), 8) +  values(9))
    y_lux = shift_in_value(y_lux, points, lux);
    
    %save the value in a variable without a ";" so we can read the number in
    %console 
    %temperature = [y_soil_temp(points) y_soil_temp(points)-273.15]
    %moisture =  y_soil_moisture(points)
    % edit just the data for the y axis on the graph. This is much, much
    % faster than ploting everything over and over again
    
    set(lh_soil_temp, 'YData',y_soil_temp);
    set(lh_soil_moisture, 'YData',y_soil_moisture);
    set(lh_air_temp, 'YData',y_air_temp);
    set(lh_air_humidity, 'YData',y_air_humidity);
    set(lh_lux, 'YData',y_lux);
    %request the plot to be updated
    drawnow;
 
end
    
close ALL %close all open figures

fclose(s); %close serial port
delete(s); %remove serial port from memory

end

function [array] = shift_in_value(array, size, value)
    %push the all the values to the left of the graph 
    for k = 1:1:size-1
        array(k) = array(k+1);
    end
    array(size) = value; 
end

function [command, size, values] = get_1_packet(s)

    
        good = 0;
        state = 0;
        check_sum = 0;
        packet_size = 0;
        counter = 1;
        command = 0;
        key = get(gcf,'CurrentKey');
        while good == 0 && strcmp(key, 's') == 0
            key = get(gcf,'CurrentKey');
            if s.BytesAvailable ~= 0 
                read = fread(s,1);
                if read == 171 && state == 0
                    state = state + 1;
                    check_sum_values(check_sum, read);
                    %check_sum = check_sum + number_of_ones(read);
                elseif state == 1
                    if read == 60
                        state = state + 1;
                        check_sum_values(check_sum, read);
                        %check_sum = check_sum + number_of_ones(read);
                    else
                        state = 0;
                        check_sum = 0;
                    end
                elseif state == 2
                    command = read;
                    check_sum_values(check_sum, read);
                    state = state + 1;
                elseif state == 3
                    size = read;
                    check_sum_values(check_sum, read);
                    state = state + 1;
                elseif state == 4
                    if counter < size
                        values(counter) = read;
                        check_sum_values(check_sum, read);
                        counter = counter +1;
                    else
                        values(counter) = read;
                        check_sum_values(check_sum, read);
                        state = state + 1;
                    end
                elseif state == 5
                        %check_sum
                        %read
                    if check_sum == read
                        state = state + 1;
                        good = 1;
                    else
                        state = 0;
                        check_sum = 0;
                    end

                end
            end
        end
end


function check_sum = check_sum_values(check_sum, values)

    check_sum = check_sum + values;
    
    if(check_sum > 65535)
        n = rem(check_sum, 65535);
        check_sum = check_sum - (65536*n);
    end
end

