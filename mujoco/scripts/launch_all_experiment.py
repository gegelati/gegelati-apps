import subprocess

def launchingTest(logFolder, environment, obstacle, paramFile, nbSeed):
    result = subprocess.run(
        ["./scripts/scriptMujocoOrga.sh", logFolder, environment, str(obstacle), paramFile, str(nbSeed - 1)],
        capture_output=True,
        text=True,
        check=True
    )
    print("Sortie standard :", result.stdout)



NO_OBSTACLE = False
SINGLE_OBSTALCE = False
NAIVE_TOURNAMENT_MAPLE = False
NAIVE_TOURNAMENT_MATPG = False
NAIVE_LEXICASE_MAPLE = False
NAIVE_LEXICASE_MATPG = False
HIERARCHICAL_TOURNAMNENT = False
HIERARCHICAL_LEXICASE = False
HIERARCHICAL_TOURNAMNENT_INDEPENDANT = False
HIERARCHICAL_LEXICASE_INDEPENDANT = True
HIERARCHICAL_TOURNAMNENT_HYBRID = False
HIERARCHICAL_LEXICASE_HYBRID = True

NB_OBSTACLE = 5

mainLog = "logs/contribTest"
environment = "half_cheetah"

if(NO_OBSTACLE):
    print("Launching No obstacle test")
    launchingTest(mainLog + "/noObstacle/", environment, "none", "params_0.json", 5)

if(SINGLE_OBSTALCE):
    for obstacle in range(NB_OBSTACLE):
        print(f"Launching singe obstacle {obstacle} test")
        launchingTest(mainLog + f"/singleObstacle_{obstacle}/", environment, obstacle, "params_0.json", 5)

if(NAIVE_TOURNAMENT_MAPLE):
    print("Launching Naive Tournament MAPLE test")
    launchingTest(mainLog + "/naive_tournament_maple/", environment, "01234", "params_1.json", 10)

if(NAIVE_TOURNAMENT_MATPG):
    print("Launching Naive Tournament MATPG test")
    launchingTest(mainLog + "/naive_tournament_matpg/", environment, "01234", "params_2.json", 10)

if(NAIVE_LEXICASE_MAPLE):
    print("Launching Naive Lexicase MAPLE test")
    launchingTest(mainLog + "/naive_lexicase_maple/", environment, "0,1,2,3,4", "params_3.json", 10)

if(NAIVE_LEXICASE_MATPG):
    print("Launching Naive Lexicase MATPG test")
    launchingTest(mainLog + "/naive_lexicase_matpg/", environment, "0,1,2,3,4", "params_4.json", 10)

if(HIERARCHICAL_TOURNAMNENT):
    print("Launching Hierarchical Tournament test")
    launchingTest(mainLog + "/hierarchical_tournament/", environment, "0,1,2,3,4", "params_5.json", 10)

if(HIERARCHICAL_LEXICASE):
    print("Launching Hierarchical Lexicase MATPG test")
    launchingTest(mainLog + "/hierarchical_lexicase/", environment, "0,1,2,3,4", "params_6.json", 10)

if(HIERARCHICAL_TOURNAMNENT_INDEPENDANT):
    print("Launching Hierarchical Tournament independant MATPG test")
    launchingTest(mainLog + "/hierarchical_tournament_indi/", environment, "0,1,2,3,4", "params_7.json", 10)

if(HIERARCHICAL_LEXICASE_INDEPENDANT):
    print("Launching Hierarchical Lexicase independant MATPG test")
    launchingTest(mainLog + "/hierarchical_lexicase_indi/", environment, "0,1,2,3,4", "params_8.json", 10)

if(HIERARCHICAL_TOURNAMNENT_HYBRID):
    print("Launching Hierarchical Tournament hybrid MATPG test")
    launchingTest(mainLog + "/hierarchical_tournament_hybrid/", environment, "0,1,2,3,4", "params_9.json", 10)

if(HIERARCHICAL_LEXICASE_HYBRID):
    print("Launching Hierarchical Lexicase hybrid MATPG test")
    launchingTest(mainLog + "/hierarchical_lexicase_hybrid/", environment, "0,1,2,3,4", "params_10.json", 10)

