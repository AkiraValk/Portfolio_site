import sys
import json
from PyQt5.QtWidgets import QApplication, QFileDialog
from PyQt5.QtGui import QColor
from Creation_Onglets import Onglets
from Creation_Camembert import Camembert
from Creation_Legendes import Legendes
from Creation_Boutons import Boutons

NB_LEGENDES_PAR_PAGE = 25


def generer_couleurs(nb_couleurs):
    return [QColor.fromHsv(i * 255 // nb_couleurs, 255, 255) for i in range(nb_couleurs)]


def generer_script_powershell(legendes_objets):
    fichiers_a_supprimer = []
    for legende in legendes_objets:
        etats = legende.recupere_etats_cases_a_cocher()
        for etat, fichier in zip(etats, legende.liste_fichiers[legende.num_legende_start:legende.num_legende_stop]):
            if etat:
                fichiers_a_supprimer.append(fichier[0])

    script = """Write-Output "Script PowerShell pour supprimer des fichiers sans confirmation"
Write-Output "Attention : cette suppression est définitive..."
$reponse = Read-Host "Veuillez confirmer la suppression de tous ces fichiers : (OUI)"
if ($reponse -eq "OUI") {
    $confirmation = Read-Host "Etes-vous bien certain(e) ? (OUI)"
    if ($confirmation -eq "OUI") {
"""

    for fichier in fichiers_a_supprimer:
        script += f'        Remove-Item -Path "{fichier}" -Force\n'

    script += """    } else {
        Write-Output "Opération annulée..."
    }
} else {
    Write-Output "Opération annulée..."
}
"""

    nom_fichier = \
    QFileDialog.getSaveFileName(None, "Enregistrer le script PowerShell", "", "PowerShell Scripts (*.ps1)")[0]
    if nom_fichier:
        with open(nom_fichier, 'w', encoding='utf-8') as f:
            f.write(script)
        print(f"Script PowerShell généré : {nom_fichier}")


def main(json_file, repertoire_base):
    with open(json_file, 'r') as f:
        liste_fichiers = json.load(f)

    app = QApplication(sys.argv)
    fenetre = Onglets()

    liste_couleurs = generer_couleurs(len(liste_fichiers))

    fromage = Camembert(liste_fichiers, liste_couleurs)
    layout_fromage = fromage.dessine_camembert()
    fenetre.add_onglet("Camembert", layout_fromage)

    legendes_objets = []

    # Répartition des légendes sur 4 onglets (25 par page)
    for num_page_legende in range(4):  # Fixé à 4 pages maximum
        start_index = num_page_legende * NB_LEGENDES_PAR_PAGE
        legendes = Legendes(liste_fichiers, liste_couleurs, start_index, nb_legende_par_page=NB_LEGENDES_PAR_PAGE)
        layout_legende = legendes.dessine_legendes()
        fenetre.add_onglet(f"Légende {num_page_legende + 1}", layout_legende)
        legendes_objets.append(legendes)

    boutons = Boutons(repertoire_base, lambda: generer_script_powershell(legendes_objets))
    layout_ihm = boutons.dessine_boutons()
    fenetre.add_onglet("IHM", layout_ihm)

    fenetre.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    if len(sys.argv) > 2:
        main(sys.argv[1], sys.argv[2])
    else:
        print("Usage: python display_results.py <json_file> <repertoire_de_base>")