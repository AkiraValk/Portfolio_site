import time
import datetime
import json

# Définir les plages horaires (heure, minute) pour activation ON
plages = [
    ((8, 0), (10, 0)),   # de 08h00 à 10h00
    ((18, 0), (22, 0))   # de 18h00 à 22h00
]

# Chemin du fichier de status partagé
STATUS_FILE = "/home/pi/prise_status.json"

def est_dans_plage(heure, minute):
    now = datetime.time(heure, minute)
    for (debut, fin) in plages:
        debut_time = datetime.time(*debut)
        fin_time = datetime.time(*fin)
        if debut_time <= now <= fin_time:
            return True
    return False

while True:
    now = datetime.datetime.now()
    actif = est_dans_plage(now.hour, now.minute)

    # Simulation : on écrit simplement le status dans un fichier json
    status = {"etat": "ON" if actif else "OFF"}
    with open(STATUS_FILE, "w") as f:
        json.dump(status, f)

    # Ici tu peux ajouter ta commande GPIO pour allumer ou éteindre la prise:
    # if actif:
    #    gpio.control_on()
    # else:
    #    gpio.control_off()

    time.sleep(30)  # rafraîchit toutes les 30 secondes
