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
s = serial('COM7', 'baudrate', 9600);
fopen(s);





%set the maximum for the values 
voltage_max = 5;
reference_voltage = 0.7568;
current_max = 100;
current_reference = 0;
lux_max = 1100;
lux_min = 0;
lux_b = 13.33; %15848916.42

points = 500; %number of points on the graph at all times
data_period = 100; %data period in milliseconds 
%x will be the time axis. The time between points is defined by
%"data_period"
x = linspace(-points*data_period,0, points);


%close all open figures
close ALL
figure


% draw a area graph with parameter x and y. Save the return value so we can
% edit the graph later. You can use "plot" instead of "area", I just liked
% the aspect better.
subplot(3,1,1);
y1 = linspace(0,0,points);
lh1 = area(x,y1, 'FaceColor', [.5 .5 .5]);
str = sprintf('Voltage, 0 to %d\%', voltage_max),
title(str);
axis([-points*data_period, 0, 0, voltage_max]);
line([-points*data_period, 0] , [ reference_voltage  reference_voltage]);

subplot(3,1,2);
y2 = linspace(0,0,points);
lh2 = area(x,y2, 'FaceColor', [.5 .5 .5]);
str = sprintf('Current, %d to %d\%', current_reference, current_max),
title(str);
axis([-points*data_period, 0, current_reference, current_max]);

subplot(3,1,3);
y3 = linspace(0,0,points);
lh3 = area(x,y3, 'FaceColor', [.5 .5 .5]);
str = sprintf('lux, %d to %d\%', lux_min, lux_max),
title(str);
axis([-points*data_period, 0, lux_min, lux_max]);
shg; %brings the figure to the front of all other windows

key = get(gcf,'CurrentKey'); %get the key currently pressed
%this "while" will stop if you press the "s" key
while ( strcmp(key, 's') == 0 ) 
    key = get(gcf,'CurrentKey'); %get the key currently pressed
    command = 0; size = 0;
    while command ~= 18 || size < 2
         [command, size, values] = get_1_packet(s);
    end
    
    %push the all the values to the left of the graph 
    for k = 1:1:points-1
        y1(k) = y1(k+1);
    end
    for k = 1:1:points-1
        y2(k) = y2(k+1);
    end
     for k = 1:1:points-1
        y3(k) = y3(k+1);
    end
       
    %Get relative humidity
    y1(points) = (bitsll(values(2), 8) +  values(1)) * (5/4096.0);

    y2(points) = (y1(points) - reference_voltage) / 0.047;
    
    y3(points) = lux_b * y2(points);
    %save the value in a variable without a ";" so we can read the number in
    %console 
 
    %disp(num2str(y2(points)*1000000,'%.3f'))
    
    disp(num2str(y3(points),'%.3f'))
    % edit just the data for the y axis on the graph. This is much, much
    % faster than ploting everything over and over again
    set(lh1, 'YData',y1);
    set(lh2, 'YData',y2);
    set(lh3, 'YData',y3);
    %request the plot to be updated
    drawnow;
 
end
    
close ALL %close all open figures

fclose(s); %close serial port
delete(s); %remove serial port from memory

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

