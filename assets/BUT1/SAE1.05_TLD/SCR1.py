from PyQt5.QtWidgets import QApplication, QFileDialog
from pathlib import Path
import sys
import os

def selectionner_repertoire():
    app = QApplication(sys.argv)
    dialog = QFileDialog()
    dialog.setFileMode(QFileDialog.Directory)
    dialog.setOption(QFileDialog.ShowDirsOnly, True)
    documents_path = os.path.expanduser("~/Documents")
    dialog.setDirectory(documents_path)
    if dialog.exec_():
        chemin = dialog.selectedFiles()[0]
        return str(Path(chemin).resolve())
    return ""



if __name__ == "__main__":
    repertoire_selectionne = selectionner_repertoire()
    print(repertoire_selectionne)
    sys.exit(0)