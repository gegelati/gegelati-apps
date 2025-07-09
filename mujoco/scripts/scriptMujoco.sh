#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <output-directory>"
    exit 1
fi

OUTPUT_PATH=$1

MEMORY="10G"
NTASKS=1
CPUS_PER_TASK=48
TIME="1296000"
OUTPUT_DIR="/data/lab_ietr/qvacher/2025MCMaster/GECCO/mujoco"

mkdir -p "$OUTPUT_PATH"
mkdir -p "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR/out"
mkdir -p "$OUTPUT_DIR/err"

folders="MAPLE MATPG TPG"
usecases="inverted_double_pendulum hopper half_cheetah walker2d ant humanoid"
seeds=$(seq 0 9)

for t in $folders; do
  for a in $usecases; do
    PARAMS_FILE="paramsGecco/$t/params_${a}_0.json"
    LOGS_FOLDER="${OUTPUT_PATH}/${t}/${a}"
    mkdir -p "$LOGS_FOLDER"

    for i in $seeds; do
      JOB_NAME_BASE="${t}_${a}_s${i}"
      job_file=$(mktemp /tmp/job_XXXX.sh)

      cat <<EOF > $job_file
#!/bin/bash
#SBATCH --mem=${MEMORY}
#SBATCH --ntasks=${NTASKS}
#SBATCH --job-name=${JOB_NAME_BASE}
#SBATCH --cpus-per-task=${CPUS_PER_TASK}
#SBATCH --exclude=crn12,crn13
#SBATCH --time=${TIME}
#SBATCH --output=${OUTPUT_DIR}/out/cout_%j.txt
#SBATCH --error=${OUTPUT_DIR}/err/cerr_%j.txt

./bin/Release/mujoco -s ${i} -p ${PARAMS_FILE} -l ${LOGS_FOLDER} -u ${a} -h 0 -g 0

EOF

      sbatch $job_file
      rm -f $job_file
    done
  done
done

echo "All jobs submitted."