
$pythonSelecteurScript = ".\SCR1.py"
$pythonMainScript = ".\SCR2.py"
$pythonDisplayScript = ".\SCR3.py"

Write-Host "Sélection du répertoire..."

$rep_base = python $pythonSelecteurScript

if (-not $rep_base) {
    Write-Host "Aucun répertoire n'a été sélectionné. Le script va se terminer."
    exit
}

if (Test-Path $rep_base -PathType Container) {
    Write-Host "Répertoire sélectionné : $rep_base"
    Write-Host "Exécution de l'analyse..."
    python $pythonMainScript $rep_base
    Write-Host "L'analyse des fichiers est terminée."
    Write-Host "Affichage des résultats..."
    python $pythonDisplayScript "fichiers_filtres.json" $rep_base

    Write-Host "L'affichage graphique est terminé."
} else {
    Write-Host "Le répertoire sélectionné n'existe pas ou n'est pas valide : $rep_base"
}
