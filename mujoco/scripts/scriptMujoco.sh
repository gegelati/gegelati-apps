#!/bin/bash

# Vérification du nombre d'arguments
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <output-directory> <usecase>"
    exit 1
fi

# Récupération de l'argument
OUTPUT_PATH=$1
USECASE=${2:-ant}
OBSTACLES=${3:-0,1,2,3,4}    # Valeur par défaut : "_"


# Paramètres pour la soumission SLURM
MEMORY="4G"
NTASKS=1
CPUS_PER_TASK=32
TIME="300"
OUTPUT_DIR="/data/lab_ietr/qvacher/obstacles/"
JOB_NAME_BASE="TPG_${USECASE,,}"


# Créer les dossier si nécessaire
mkdir -p "$OUTPUT_PATH"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR/out"
mkdir -p "$OUTPUT_DIR/err"


list_params=$(seq 0 0 | sed "s/^/params_/" | sed 's/$/.json/')
seeds=$(seq 0 4)

# Boucle sur chaque ensemble de paramètres
for params in ${list_params[@]}; do
    echo "Params used $params"

    # Extraire l'index du fichier JSON
    index_param=$(basename "$params" | grep -oE '[0-9]+' | tail -n1)

    for i in ${seeds[*]}; do
        echo "Seed used $i"

        # Crée un fichier temporaire pour la soumission SLURM
        job_file=$(mktemp /tmp/job_XXXX.sh)

        # Écrire le contenu du job SLURM dans le fichier temporaire
        cat <<EOF > $job_file
#!/bin/bash
#SBATCH --mem=${MEMORY}
#SBATCH --ntasks=${NTASKS}
#SBATCH --job-name=${JOB_NAME_BASE}_s${i}_p${index_param}
#SBATCH --cpus-per-task=${CPUS_PER_TASK}
#SBATCH --exclude=crn12,crn13
#SBATCH --time=${TIME}
#SBATCH --output=${OUTPUT_DIR}/out/cout_%j.txt
#SBATCH --error=${OUTPUT_DIR}/err/cerr_%j.txt

# Exécution du programme avec les paramètres
./bin/Release/mujoco -s ${i} -p ${params} -l ${OUTPUT_PATH} -u ${USECASE}  -g 0 -o 1 -h 0 -w ${OBSTACLES}

EOF

        # Soumettre le job SLURM
        sbatch $job_file

        # Nettoyer le fichier temporaire après soumission
        rm -f $job_file

    done
done

echo "All jobs submitted."