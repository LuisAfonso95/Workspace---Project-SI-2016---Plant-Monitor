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
%while 1
%end
% Init and open the serial port
s = serial('COM7', 'baudrate', 9600);
fopen(s);





%set the maximum for the values 
humidity_max = 100;
temperature_max = 40;


points = 10; %number of points on the graph at all times
data_period = 96; %data period in milliseconds 
%x will be the time axis. The time between points is defined by
%"data_period"
x = linspace(-points*data_period,0, points);

%y will hold the distance, for now all values will be 0 and will have the
%size defined by "points"
y1 = linspace(0,0,points);

%close all open figures
close ALL
figure

subplot(2,1,1);

% draw a area graph with parameter x and y. Save the return value so we can
% edit the graph later. You can use "plot" instead of "area", I just liked
% the aspect better.
lh1 = area(x,y1, 'FaceColor', [.5 .5 .5]);
str = sprintf('Humidity, 0 to %d\%', humidity_max),
title(str);
axis([-points*data_period, 0, 0, humidity_max]);


subplot(2,1,2);
y2 = linspace(0,0,points);
lh2 = area(x,y2, 'FaceColor', [.5 .5 .5]);
str = sprintf('Temperature, 0 to %dmºC', temperature_max);
title(str);
axis([-points*data_period, 0, 0, temperature_max]);

shg; %brings the figure to the front of all other windows

key = get(gcf,'CurrentKey'); %get the key currently pressed
%this "while" will stop if you press the "s" key
while ( strcmp(key, 's') == 0 ) 
    key = get(gcf,'CurrentKey'); %get the key currently pressed
    command = 0; size = 0;
    while command ~= 18 || size < 4
         [command, size, values] = get_1_packet(s);
    end
    
    %push the all the values to the left of the graph 
    for k = 1:1:points-1
        y1(k) = y1(k+1);
    end
    for k = 1:1:points-1
        y2(k) = y2(k+1);
    end  
    
    %Get relative humidity
    y1(points) = (bitsll(values(2), 8) +  values(1));
    temp1 = bitand( y1(points), 3)
    y1(points) =  bitand(y1(points),65532);
    y1(points) =   -6.0 + 125.0/65536 * y1(points);
    
    %Get temperature
    y2(points) = (bitsll(values(4), 8) +  values(3));
    temp2 = bitand( y2(points), 3)
    y2(points) =  bitand(y2(points),65532);
    y2(points) =  -46.85 + (175.72 * (y2(points) / 65536.0));

    %save the value in a variable without a ";" so we can read the number in
    %console 
    value1 = [y1(points)  y2(points)]
    
    % edit just the data for the y axis on the graph. This is much, much
    % faster than ploting everything over and over again
    set(lh1, 'YData',y1);
    set(lh2, 'YData',y2);
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

