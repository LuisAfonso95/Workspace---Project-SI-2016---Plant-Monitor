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
y_max = 4096;


points = 20000; %number of points on the graph at all times
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


% draw a area graph with parameter x and y. Save the return value so we can
% edit the graph later. You can use "plot" instead of "area", I just liked
% the aspect better.
lh1 = area(x,y1, 'FaceColor', [.5 .5 .5]);
str = sprintf('0 to %d\%', y_max),
title(str);
axis([-points*data_period, 0, 0, y_max]);
line([-points*data_period, 0] , [ 2048 2048]);

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

    
    %Get relative humidity
    y1(points) = (bitsll(values(2), 8) +  values(1));


    %save the value in a variable without a ";" so we can read the number in
    %console 
    value1 = [y1(points)]
    
    % edit just the data for the y axis on the graph. This is much, much
    % faster than ploting everything over and over again
    set(lh1, 'YData',y1);
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
                elif state == 1
                    if read == 60
                        state = state + 1;
                        check_sum_values(check_sum, read);
                        %check_sum = check_sum + number_of_ones(read);
                    else
                        state = 0;
                        check_sum = 0;
                    end
                elif state == 2
                    command = read;
                    check_sum_values(check_sum, read);
                    state = state + 1;
                elif state == 3
                    size = read;
                    check_sum_values(check_sum, read);
                    state = state + 1;
                elif state == 4
                    if counter < size
                        values(counter) = read;
                        check_sum_values(check_sum, read);
                        counter = counter +1;
                    else
                        values(counter) = read;
                        check_sum_values(check_sum, read);
                        state = state + 1;
                    end
                elif state == 5
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

