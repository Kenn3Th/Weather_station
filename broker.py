import time
import RPi.GPIO as GPIO 
import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import numpy as np
import datetime

# Funksjon som lager plott
plt.style.use('ggplot')
def plot_graf(data,fig_nr,enhet,verdier):
    print("plotter", data)
    tid = [datetime.datetime.now() + datetime.timedelta(seconds=i) 
           for i in range(len(verdier))]
    x_verdier = np.linspace(0, len(verdier)/2, len(verdier))
    plt.xticks(np.arange(10) ,x_verdier)
    plt.figure(fig_nr, clear=True)
    if fig_nr==1:  
       plt.ylim(-30,40)
    elif fig_nr==2:
       plt.ylim(20,60)
    else:
       plt.ylim(0,20) 
    plt.ylabel(enhet)
    plt.gcf().autofmt_xdate()
    plt.title(data)
    plt.plot(x_verdier, verdier)
    plt.savefig(data+".png")
    plt.show()

# Setup "callback" funksjon som blir kalt naar noe skjer paa MQTT nettverket 
# eks: kobler seg til serveren eller mottar en beskjed fra en klient 
def on_connect(client, userdata, flags, rc): 
   print("Connected with result code " + str(rc)) 
   # Abonnerer paa on_connect() betyr at man kobler seg til serveren
   # og den kobler til paa nytt hvis tilkoblingen blir brutt.
   client.subscribe("/ute/pi")
   # Henter data fra klienter
#Tomme lister som lagrer motatt data.   
Temp = [] 
Humi = []
Vind = []
# "callback" for naar en PUBLISH beskjed blir mottatt fra en "publisher" 
def on_message(client, userdata, msg): 
    global timer
    if msg.topic == '/ute/pi':
        data = float(msg.payload) #motatt data
        if data>1000:
           Data = data-1000
           Vind.append(Data)
           print("Vind: "+str("%0.2f" % Data))
        elif 1000>data>400:
           Data = data-500
           Temp.append(Data)
           print("Temp: "+str("%0.2f" % Data))
        else:
           Humi.append(data)
           print("Fukt: "+str("%0.2f" % data))

    if len(Vind)>5:
        print("Like a glove!")
        plot_graf("Temperatur",1,"C",Temp)
        plot_graf("Fuktighet",2,"%",Humi)
        plot_graf("Vindhastighet",3,"m/s",Vind)
        send_data = str("%0.2f" % Temp[-1])+'/'+str("%0.2f" % Humi[-1])+'/'+str("%0.2f" % Vind[-1])
        print(send_data)
        client.publish('/info/esp8266',send_data)

# Lager MQTT klienter og kobler de til den "localhost"
# "localhost": Raspberry PI som kjorer denne koden
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message 
client.connect('localhost', 1883, 60)   
print('Script is running, press Ctrl-C to quit...')
# Setter opp en klient-loop som gaar helt tl den manuelt blir frakoblet
client.loop_forever()
