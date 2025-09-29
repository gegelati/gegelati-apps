import subprocess
import re
import pandas as pd
import warnings
from typing import Optional, List, Dict, Any
from concurrent.futures import ThreadPoolExecutor, as_completed
from tqdm import tqdm  # Pour une barre de progression plus propre

def run_mujoco_and_get_score(env: str, w: int, wTest: int, sTrain: int, sTest: int) -> Optional[Dict[str, Any]]:
    """Exécute la commande Mujoco et retourne un dictionnaire avec les métadonnées + score (ou None)."""
    cmd = [
        "./bin/Release/renderMujoco",
        "-u", env,
        "-d", f"logs/{env}/oneObstacle_{w}/out_best.{sTrain}.p0.{env}.dot",
        "-w", str(wTest),
        "-v", "2",
        "-s", str(sTest)
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        match = re.search(r"Score:\s+([\d.-]+)\s+And utility ([\d.-]+)", result.stdout)
        if match:
            return {
                "environment": env,
                "obstacleTrain": w,
                "obstacleTest": wTest,
                "seedTraining": sTrain,
                "seedTesting": sTest,
                "score": float(match.group(1))
            }
        else:
            print(f"\nSortie inattendue pour {env}, w={w}, sTrain={sTrain}, sTest={sTest}")
            print(result.stdout)
            return None
    except subprocess.CalledProcessError as e:
        print(f"\nErreur avec {env}, w={w}, sTrain={sTrain}, sTest={sTest}:")
        print(e.stderr)
        return None

def generate_tasks(envs: List[str], nb_obsTrain: int, nb_obsTest: int, nb_seed_train: int, nb_seed_test: int) -> List[tuple]:
    """Génère toutes les combinaisons de paramètres pour les tâches."""
    tasks = []
    for env in envs:
        for obs in range(nb_obsTrain):
            for obsTest in range(nb_obsTest):
                for sTrain in range(nb_seed_train):
                    for sTest in range(nb_seed_test):
                        tasks.append((env, obs, obsTest, sTrain, sTest))
    return tasks

def main():
    # Configuration
    envs = ["half_cheetah"]
    nb_obsTrain = 5
    nb_obsTest = 5
    nb_seed_train = 5
    nb_seed_test = 10
    max_workers = 8  # Ajuste selon ta machine (ex: 4-16)

    # Initialisation du DataFrame
    df = pd.DataFrame(columns=[
        "environment", "obstacleTrain", "obstacleTest",
        "seedTraining", "seedTesting", "score"
    ])

    # Génération des tâches
    tasks = generate_tasks(envs, nb_obsTrain, nb_obsTest, nb_seed_train, nb_seed_test)
    total_tasks = len(tasks)

    # Exécution multi-threadée
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # Soumission des tâches
        futures = {executor.submit(run_mujoco_and_get_score, *task): task for task in tasks}

        # Barre de progression + traitement des résultats
        with tqdm(total=total_tasks, desc="Progression") as pbar:
            for future in as_completed(futures):
                task = futures[future]
                try:
                    result = future.result()
                    if result is not None:
                        df = pd.concat([df, pd.DataFrame([result])], ignore_index=True)
                        # Sauvegarde incrémentale (toutes les 10 tâches pour éviter l'I/O excessif)
                        if len(df) % 10 == 0:
                            df.to_csv("logs/dataOneObstacle.csv", index=False)
                except Exception as e:
                    print(f"\nErreur inattendue pour la tâche {task}: {e}")
                finally:
                    pbar.update(1)

    # Sauvegarde finale
    print("\n" + "="*50)
    print("Résultats finaux:")
    print(df.describe())
    print(f"\nSauvegarde de {len(df)} résultats dans logs/dataOneObstacle.csv")
    df.to_csv("logs/dataOneObstacle.csv", index=False)

if __name__ == "__main__":
    warnings.filterwarnings("ignore", category=FutureWarning)
    main()
