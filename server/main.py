import time
import paho.mqtt.client as paho
import mysql.connector
from mysql.connector import Error

#this defines the address of the broker, in this case it is localy hosted.
broker="localhost"

#defines connection to (myql) data base
mydb = mysql.connector.connect(
  host="localhost",
  user="app",
  password="app",
  database="LaserTag"
)
mycursor = mydb.cursor()

#This function defiens the on message function (handle incomming mqtt messages)
def on_message(client, userdata, message):
    message = str(message.payload.decode("utf-8")) #this decodes the message
    print("received message =",message)
    record = message.split('#')                    #this splits the massage on each "#" this is because the data is parced by the "#" symbol.
    
    sql = "INSERT INTO messages (totalBullets, currentHealth, espID) VALUES (%s, %s)" #here the sql statement format is constructed.
    val = (str(record[3]), str(record[1]),str(record[0]))
    mycursor.execute(sql, val)                     #here the data is inserted into the database.
    mydb.commit()
    print("added to DB")
    
    if len(record)<4:    
        sql = "INSERT INTO shotBy (espGotShot, espShoot) VALUES (%s, %s)"
        val = (str(record[0]), str(record[4]))
        mycursor.execute(sql, val)
        mydb.commit()
        print("added to DB")
    

#redundent "correct" way to do this?
def connect():
    """ Connect to MySQL database """
    conn = None
    try:
        conn = mysql.connector.connect(host='localhost',
                                       database='LaserTag',
                                       user='app',
                                       password='app')
        if conn.is_connected():
            print('Connected to MySQL database')

    except Error as e:
        print(e)

    finally:
        if conn is not None and conn.is_connected():
            conn.close()



client= paho.Client("client-001") #create client object client1.on_publish = on_publish #assign function to callback client1.connect(broker,port) #establish connection client1.publish("house/bulb1","on")
#Bind function to callback
client.on_message=on_message



print("connecting to broker ",broker)
client.connect(broker) #connect to broker (in this case a local broker)
client.loop_start()    #start loop to process received messages


#loop to subscribe to all the channels (this could be improved by using a nested function where it querries the data base on wich esp's are in the game (and thus also using a channel)). 
print("subscribing ")
for i in range(10):
    client.subscribe(str(i))


#loop to publish to all the channels (this could be improved by using a nested function where it querries the data base on wich esp's are in the game (and thus also using a channel)). 
#currently it first runs a sql querry to check what data has been configured on the dashboard, and then sends the correct data to the correct gun.
print("publishing ")
for i in range(10):
    sql_select_Query = "select espTeam, espDamage from esp where espID =" + str(i)
    cursor = mydb.cursor()
    cursor.execute(sql_select_Query)
    # get all records
    #records = cursor.fetchall()
    print("Total number of rows in table: ", cursor.rowcount)    
    print("\nPrinting each row")
    for row in cursor.fetchall():
        print("espTeam = ", row.espTeam )
        print("name = ", row.espDamage, "\n")
        temp = str(row.espTeam) + str(row.espDamage) #here it sends the configuration data of for the gun: the team the gun belongs to, and the bullet type: (a preset type that signals a set amount of dammage and cooldown).
        client.publish(str(i), str(temp))





# this is present in nearly all examples, but breacks the code when it is uncommented:
#  
# client.disconnect() #disconnect
# client.loop_stop() #stop loop
