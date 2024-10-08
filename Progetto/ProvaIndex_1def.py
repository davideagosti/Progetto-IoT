from flask import Flask, render_template, request, redirect, url_for
from database import mysql
#from DB_Operations import get_data
import mysql.connector


myDb = mysql.connector.connect(
    host="192.168.137.147",   #raspberrypi
    user="pythonUser",
    password="pythonPWD",
    database="tuoDB"
)


myCursor = myDb.cursor()# riferimento all interno del DB, cursore

myCursor.execute("SHOW DATABASES")
if not myCursor.fetchone():
    print("Nessuna DB trovato")
else:
    print("Database Trovati")
    for x in myCursor:
        print(x)# funzione ciclica per vedere i Databases
        myCursor.execute(("SHOW TABLES"))
        for table in myCursor:
            print("Tabelle Trovate")
            print(table)# funzione ciclica per vedere tutte le tabelle se ho i DB
        #else:
         #   print("Non ci sono Tabelle")

sqlVis = "SELECT * FROM Records"

def get_data():# funzione che mi restituisce le righe del db da mettere in tabella nella pagina index
    myCursor.execute(sqlVis)
    row = myCursor.fetchall()
    return row

app = Flask(__name__)#----------------------------------------------------------------------------

print(f"Connesso al database: {myDb.database}")
print(f"Esecuzione query: {sqlVis}")

myCursor.execute("SHOW TABLES;")
print(myCursor.fetchall())

myDb.autocommit = True

@app.route("/provaindex")# funzione che abilita una root cioè una pagina html dove visualizzare i dati del db nel rasp
def provaindex():

    row = get_data()
    return render_template("indexprovaindex_1.html", righe = row)

@app.route("/prova")# funzione che abilita una root cioè una pagina html dove visualizzare i dati del db nel rasp
def prova():

    row = get_data()
    return render_template("prov.html")

    
if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=8000)
