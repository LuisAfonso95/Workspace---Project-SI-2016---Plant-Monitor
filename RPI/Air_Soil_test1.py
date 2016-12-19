
def check_sum_values(check_sum, values):

	#print "Checking"
	check_sum = check_sum + values

	if(check_sum > 65535):
		n = check_sum % 65535
		check_sum = check_sum - (65536*n)

	return


def get_1_packet(s):
	good = 0;
	state = 0;
	check_sum = 0;
	packet_size = 0;
	counter = 0;
	command = 0;

	while good == 0:
		if s.inWaiting() != 0 :
			read = s.read(1)#fread(s,1);
			read = ord(read)
			#print  "%d" % (ord(read));
			if read == 171 and state == 0 :
				#print "Received Sync1"
				state = state + 1;
				check_sum_values(check_sum, read);
			elif state == 1 :
				if read == 60:
					state = state + 1;
					check_sum_values(check_sum, read);
				else:
					state = 0;
					check_sum = 0;
			elif state == 2:
				command = read;
				check_sum_values(check_sum, read);
				state = state + 1;
			elif state == 3:
				size = read;
				check_sum_values(check_sum, read);
				state = state + 1;
				values = [0] * size
				#print  "%d" % (size);
			elif state == 4:
				#print  "Counter = %d" % (counter);
				if counter < size-1:
					values[counter] = read;
					check_sum_values(check_sum, read);
					counter = counter +1;
				else:
					values[counter] = read;
					check_sum_values(check_sum, read);
					state = state + 1;
			elif state == 5:
				if check_sum == read:
					state = state + 1;
					good = 1;
				else:
					state = 0;
					check_sum = 0;

	#print "Received a packet"
	return values

			
import httplib, urllib
import time
import serial
key = 'XQMOQNFEIT97KAZS'  # Thingspeak channel to update	
def send_stuff(soil_temperature, soil_moisture, air_temperature, air_r_humidity):
    while True:
        #Calculate CPU temperature of Raspberry Pi in Degrees C
        params = urllib.urlencode({'field1': soil_temperature, 'field2': soil_moisture, 'field3': air_temperature, 'field4': air_r_humidity,'key':key }) 
        headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
        conn = httplib.HTTPConnection("api.thingspeak.com:80")
        try:
            conn.request("POST", "/update", params, headers)
            response = conn.getresponse()
            print response.status, response.reason
            data = response.read()
            conn.close()
        except:
            print "connection failed"
        break

if __name__ == '__main__':
	ser = serial.Serial ("/dev/ttyUSB0")    #Open named port 
	ser.baudrate = 9600                     #Set baud rate to 9600
   
   
	while True:
		values = get_1_packet(ser)
		converted = [0] * (len(values)/2)
		
		for x in range(0, len(values)/2):
			converted[x] = (values[(2*x)+1]* pow(2, 8)) + values[2*x]
			#print "%d shifted and or %d gives %d" %(values[(2*x)+1], values[2*x], converted[x] )
		
		
		
		soil_temperature = (converted[0]/100.0) - 273.15
		soil_moisture = converted[1]
		air_temperature = converted[2]/100.0
		air_r_humidity = converted[3]/100.0
		print " soil_temperature = %d C" % (soil_temperature)
		print " soil_moisture = %d" % (soil_moisture)
		print " air_temperature = %d C" % (air_temperature)
		print " air_r_humidity = %d" % (air_r_humidity)
		print "Sending stuff to cloud"
		send_stuff(soil_temperature, soil_moisture, air_temperature, air_r_humidity)
	ser.close()   




 