import sys
from pathlib import Path
import json
def lister_fichiers(repertoire_de_base):
    liste_fichiers = []
    chemin_base = Path(repertoire_de_base).resolve()
    for fichier in chemin_base.rglob('*'):
        if fichier.is_file():
            liste_fichiers.append([str(fichier.absolute()), fichier.stat().st_size])
    return liste_fichiers
def trier_fichiers_par_taille(liste_fichiers):
    return sorted(liste_fichiers, key=lambda x: x[1], reverse=True)

def filtrer_fichiers(liste_fichiers, TAILLE_MINI_FICHIER_EN_MEGA_OCTET, NB_MAXI_FICHIERS):
    taille_mini_octets = TAILLE_MINI_FICHIER_EN_MEGA_OCTET * 1048576
    return [f for f in liste_fichiers if f[1] > taille_mini_octets][:NB_MAXI_FICHIERS]

def creer_fichier_json(liste_fichiers, nom_fichier_json):
    liste_fichiers_modifiee = [[f[0].replace('\\', '\\\\'), f[1]] for f in liste_fichiers]
    with open(nom_fichier_json, 'w', encoding='utf-8') as f:
        json.dump(liste_fichiers_modifiee, f, indent=4)

def main():
    if len(sys.argv) < 2:
        print("Erreur : Veuillez spécifier le répertoire de base.")
        sys.exit(1)

    repertoire_de_base = sys.argv[1]
    TAILLE_MINI_FICHIER_EN_MEGA_OCTET = 1
    NB_MAXI_FICHIERS = 100  # Limitation stricte à 100 fichiers
    NOM_FICHIER_JSON = "fichiers_filtres.json"

    liste_fichiers = lister_fichiers(repertoire_de_base)
    liste_triee = trier_fichiers_par_taille(liste_fichiers)
    liste_filtree = filtrer_fichiers(liste_triee, TAILLE_MINI_FICHIER_EN_MEGA_OCTET, NB_MAXI_FICHIERS)

    creer_fichier_json(liste_filtree, NOM_FICHIER_JSON)

    print(f"Fichiers de plus de {TAILLE_MINI_FICHIER_EN_MEGA_OCTET} Mo (max {NB_MAXI_FICHIERS} fichiers) :")
    for fichier, taille in liste_filtree:
        print(f"Fichier: {fichier}, Taille: {taille} octets")
    print(f"Le fichier JSON '{NOM_FICHIER_JSON}' a été créé avec succès.")

if __name__ == "__main__":
    main()