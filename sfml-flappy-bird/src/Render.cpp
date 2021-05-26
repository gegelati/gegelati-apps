//
// Created by thomas on 19/05/2021.
//
#include "Render.h"


const sf::String Render::Display::title = "Floppy Bird";

Render::Display::Display(): window(sf::VideoMode(width, height), title){

    // set general settings of the window.
	window.setFramerateLimit(frameRate);
	window.setKeyRepeatEnabled(false);
}

sf::RenderWindow &Render::Display::getWindow()  {
    return window;
}

void Render::controllerLoop(std::atomic<bool> &exit, std::atomic<bool> &toggleDisplay, std::atomic<bool> &doDisplay,
                            const TPG::TPGVertex **bestRoot, const Instructions::Set &set,
                            Flappy_bird &flappyBirdLE,
                            const Learn::LearningParameters &params, std::atomic<uint64_t> &generation) {
    Display d = Display();
    exit = false;

    // Prepare objects for replays
    Environment env(set, flappyBirdLE.getDataSources(), params.nbRegisters, params.nbProgramConstant);
    TPG::TPGExecutionEngine tee(env);
    uint64_t frame = 0;
    std::deque<std::tuple<uint64_t, sf::Sprite>> replay;

    while (!exit) {

        // Was a replay requested?
        if (doDisplay) {
            // Compute all moves
            replay.clear();
            flappyBirdLE.reset(0, Learn::LearningMode::VALIDATION);
            for (auto action = 0; action < params.maxNbActionsPerEval; action++) {
                auto vertexList = tee.executeFromRoot(**bestRoot);
                const auto actionID = ((const TPG::TPGAction*)vertexList.back())->getActionID();
                replay.push_back(std::make_tuple(action, flappyBirdLE.getSprite()));
                flappyBirdLE.doAction(actionID);

            }
            // moves computed doDisplay = false <=> replay has been created
            doDisplay = false;
        }

        if (!replay.empty()) {
            frame = std::get<0>(replay.front());
            replay.pop_front();
        }
        d.getWindow().draw((sf::Sprite)(std::get<1>(replay.front())));
        d.getWindow().display();

        sf::Event event;
        d.getWindow().pollEvent(event);
        switch (event.type) {
            case sf::Event::KeyPressed :
                if (event.key.code == sf::Keyboard::T) {
                    std::cout << std::endl << "Display " << ((toggleDisplay) ? "de" : "re") << "activated."
                              << std::endl;
                    toggleDisplay = !toggleDisplay;
                    break;
                }
                else if(event.key.code == sf::Keyboard::Q) {
                    exit = true;
                    doDisplay = false;
                    break;
                }
            default:
                // Nothing to do
                break;
        }
        //TODO la fin du programme est mal géré : ferme la fenêtre et envoie le message mais ne quitte pas après la prochaine génération
    }

    printf("\nProgram will terminate at the end of next generation.\n");
    std::cout.flush();
}


