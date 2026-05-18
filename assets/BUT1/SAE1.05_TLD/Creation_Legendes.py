#--------------------------------------------------------
# Script : Creation_Legendes.py
# Destiné à la SAE 1.05 : traitement des données
# Dev : O. ECKLE - Ver : 1.0 - Décembre 2024
#--------------------------------------------------------

from PyQt5.QtWidgets import QWidget, QVBoxLayout,  QLabel, QHBoxLayout, QCheckBox
from PyQt5.QtCore import Qt

class Legendes:
    """
    Création d'un objet graphique contenant une série de lignes de légendes.
    La série de données représente la liste passée dans le 1er argument.
    Les couleurs des tranches du camembert sont celle de la liste passée en 2ème argument.
    Le nombre de lignes de légendes est passé dans le 4ème argument.
    Le numéro de la 1ère ligne de légendes pris dans la liste des données est passé dans le 3ème argument...
    """
    def __init__(self, liste_fichiers, liste_couleurs, num_legende_start, nb_legende_par_page):
        self.liste_fichiers = liste_fichiers
        self.liste_couleurs = liste_couleurs
        self.num_legende_start = num_legende_start
        self.nb_legende_par_page = nb_legende_par_page
        self.num_legende_stop = min(len(liste_fichiers), num_legende_start+self.nb_legende_par_page)
        self.cases_a_cocher = []

    def dessine_legendes(self):
        """
        Retourne une Widget Layout PyQt contenant une liste de légendes.
        """

        # Création de l'objet graphique et son layout contenant un certain nombre de lignes de légendes
        legendes = QWidget()
        zone_legendes = QVBoxLayout(legendes)
        zone_legendes.setContentsMargins(0, 20, 0, 20)
        zone_legendes.setAlignment(Qt.AlignTop)

        # Ajouter des datas à chaque légende
        for num_slice in range(self.num_legende_start, self.num_legende_stop):
            # Création de la case à cocher
            case_a_cocher = QCheckBox()
            case_a_cocher.setChecked(False)  # Définir l'état initial de la case (cochée ou non)
            self.cases_a_cocher.append(case_a_cocher)

            # Création d'un petit carré coloré
            rectangle_colore = QWidget()
            rectangle_colore.setFixedSize(16, 16)  # Taille du carré
            couleur = self.liste_couleurs[num_slice].name()
            rectangle_colore.setStyleSheet(f"background-color: {couleur}; border: 1px solid black;")

            # Composition du contenu textuel de la légende
            chemin_fichier = self.liste_fichiers[num_slice][0]  # Extrait le chemin et nom du fichier
            taille_fichier = self.liste_fichiers[num_slice][1] // 1048576  # Extrait la taille du fichier
            etiquette_legende = f"<span style='color:black;font-family:Arial Narrow'>{chemin_fichier} --> (</span><span style='color:red;'>{taille_fichier} Mo</span>)"
            texte_legende = QLabel(etiquette_legende)

            # Assemblage des éléments d'une ligne de légende
            ligne_legende = QHBoxLayout()
            ligne_legende.setAlignment(Qt.AlignLeft)
            ligne_legende.setContentsMargins(5, 0, 5, 5)
            ligne_legende.stretch(2)
            ligne_legende.addWidget(case_a_cocher)
            ligne_legende.addWidget(rectangle_colore)
            ligne_legende.addWidget(texte_legende)

            # Création d'une ligne de légende
            legende = QWidget()
            legende.setLayout(ligne_legende)
            legende.setContentsMargins(0, 0, 0, 0)

            zone_legendes.addWidget(legende)
            zone_legendes.setSpacing(0)

        # Retour de l'objet graphique et son layout contenant 25 lignes de légendes
        return legendes

    def recupere_etats_cases_a_cocher(self):
        """
        Retourne une liste de boolean indiquant, pour cette page de légendes (cet objet), les états des cases à cocher...
        """
        etats = []
        for case in self.cases_a_cocher:
            etats.append(case.isChecked())
        return etats

