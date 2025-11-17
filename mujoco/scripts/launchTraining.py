

import subprocess

def launchingTest(logFolder, environment, descriptors, paramFile, nbSeed):
    result = subprocess.run(
        ["./scripts/scriptMujocoOrga.sh", logFolder, environment, str(descriptors), paramFile, str(nbSeed - 1)],
        capture_output=True,
        text=True,
        check=True
    )
    print("Sortie standard :", result.stdout)

TOURNAMENT_CLASSIC_HALF = True
TOURNAMENT_CLASSIC_ALL = True
TOURNAMENT_CROSSOVER = True
TOURNAMENT_CROSSOVER_FULL = True

MAP_ELITES_FEET_CONTACT = False
MAP_ELITES_ACTION_VALUES = False
MAP_ELITES_NB_INSTR = False
MAP_ELITES_ACTION_VALUES__NB_INSTR = False

mainLog = "logs/"
environments = [
    #"inverted_double_pendulum",
    "hopper",
    "walker2d",
    "halfcheetah",
    "ant",
    "humanoid"
]
nbSeed = 10

for environment in environments:
    if(TOURNAMENT_CLASSIC_HALF):
        print(f"Launching TOURNAMENT_CLASSIC_HALF test for {environment}")
        launchingTest(mainLog + f"{environment}/TOURNAMENT_CLASSIC_HALF/", environment, "none", f"params/{environment}/params_0.json", nbSeed)
    if(TOURNAMENT_CLASSIC_ALL):
        print(f"Launching TOURNAMENT_CLASSIC_ALL test for {environment}")
        launchingTest(mainLog + f"{environment}/TOURNAMENT_CLASSIC_ALL/", environment, "none", f"params/{environment}/params_1.json", nbSeed)
    if(TOURNAMENT_CROSSOVER):
        print(f"Launching TOURNAMENT_CROSSOVER test for {environment}")
        launchingTest(mainLog + f"{environment}/TOURNAMENT_CROSSOVER/", environment, "none", f"params/{environment}/params_2.json", nbSeed)
    if(TOURNAMENT_CROSSOVER_FULL):
        print(f"Launching TOURNAMENT_CROSSOVER_FULL test for {environment}")
        launchingTest(mainLog + f"{environment}/TOURNAMENT_CROSSOVER_FULL/", environment, "none", f"params/{environment}/params_3.json", nbSeed)

    if(MAP_ELITES_FEET_CONTACT):
        print(f"Launching MAP_ELITES_FEET_CONTACT test for {environment}")
        launchingTest(mainLog + f"{environment}/MAP_ELITES_FEET_CONTACT/", environment, "FeetContact", f"params/{environment}/params_4.json", nbSeed)
    if(MAP_ELITES_ACTION_VALUES):
        print(f"Launching MAP_ELITES_ACTION_VALUES test for {environment}")
        launchingTest(mainLog + f"{environment}/MAP_ELITES_ACTION_VALUES/", environment, "ActionValues", f"params/{environment}/params_4.json", nbSeed)
    if(MAP_ELITES_NB_INSTR):
        print(f"Launching MAP_ELITES_NB_INSTR test for {environment}")
        launchingTest(mainLog + f"{environment}/MAP_ELITES_NB_INSTR/", environment, "NbInstr", f"params/{environment}/params_4.json", nbSeed)
    if(MAP_ELITES_ACTION_VALUES__NB_INSTR):
        print(f"Launching MAP_ELITES_ACTION_VALUES__NB_INSTR test for {environment}")
        launchingTest(mainLog + f"{environment}/MAP_ELITES_ACTION_VALUES__NB_INSTR/", environment, "ActionValues,NbInstr", f"params/{environment}/params_4.json", nbSeed)