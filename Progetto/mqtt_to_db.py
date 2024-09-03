import mysql.connector
import paho.mqtt.client as mqtt

# Configurazione del database
db_config = {
    'host': '192.168.137.147',  # IP del server MariaDB
    'user': 'pythonUser',
    'password': 'pythonPWD',
    'database': 'tuoDB'
}

# Funzione per inserire i dati nel database
def insert_into_db(data):
    try:
        connection = mysql.connector.connect(**db_config)
        cursor = connection.cursor()

        # Prepara la query SQL
        query = """
        INSERT INTO Records (mac_address, sensor_id, node_id, date, time, temperature, humidity)
        VALUES (%s, %s, %s, %s, %s, %s, %s)
        """
        print(type(data[3]))
        print((data[3]))
        print(type(data[4]))
        print((data[4]))

        cursor.execute(query, data)
        connection.commit()

        print("Dati inseriti con successo nel database.")
    except mysql.connector.Error as err:
        print(f"Errore: {err}")
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

# Funzione di callback per gestire i messaggi MQTT ricevuti
def on_message(client, userdata, message):
    msg = message.payload.decode('utf-8')
    print(f"Messaggio ricevuto: {msg}")

    # Parsing del messaggio
    try:
        parts = msg.split(';')
        if len(parts) == 8:  # 7 dati + un elemento vuoto alla fine
            mac_address = parts[0]
            sensor_id = int(parts[1])
            node_id = int(parts[2])
            date = parts[3]
            time = parts[4]
            temperature = float(parts[5])
            humidity = float(parts[6])

            # Inserisci i dati nel database
            insert_into_db((mac_address, sensor_id, node_id, date, time, temperature, humidity))
        else:
            print("Formato del messaggio non valido.")
    except Exception as e:
        print(f"Errore durante il parsing o l'inserimento nel database: {e}")

# Configura il client MQTT
mqtt_client = mqtt.Client()
mqtt_client.username_pw_set("public", "public")
mqtt_client.on_message = on_message

# Connessione al broker MQTT
mqtt_client.connect("public.cloud.shiftr.io", 1883, 60)

# Iscriviti al topic
mqtt_client.subscribe("/mqttPubSub")

# Loop infinito per ascoltare i messaggi
mqtt_client.loop_forever()
