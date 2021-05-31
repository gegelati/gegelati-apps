//
// Created by thomas on 19/05/2021.
//
#include "Render.h"



const sf::String Render::Display::title = "Floppy Bird";
const std::chrono::duration<int,std::milli> Render::Display::deltaTime = std::chrono::milliseconds(30);

Render::Display::Display(): window(sf::VideoMode(width, height), title), gameInit(), generationText(){
    // set general settings of the window.
	generationText.setFont(gameInit.getFont());
    generationText.setFillColor(sf::Color::White);
    generationText.move(textGenerationX, textGenerationY);
    generationText.setString("GEN "+std::to_string(0));

	frameText.setFont(gameInit.getFont());
    frameText.setFillColor(sf::Color::White);
    frameText.move(textFrameX, textFrameY);
    //frameText.setString("LEARNING");
    frameText.setString("FRAME " + std::to_string(0));

    window.setFramerateLimit(frameRate);
	window.setKeyRepeatEnabled(false);

    flappySprite.setTexture(gameInit.getTextures().getFlappy(1));
    flappySprite.setPosition(FlappyX, initFlappyY);
    flappySprite.setScale(flappyScale, flappyScale);


    window.clear();
    window.draw(gameInit.getBackground()[0]);
    window.draw(gameInit.getBackground()[1]);
    window.draw(gameInit.getBackground()[2]);
    window.draw(flappySprite);

    // draw scores
    window.draw(gameInit.scoreText);
    window.draw(gameInit.highscoreText);
    window.draw(generationText);
    window.draw(frameText);
    // display
    window.display();
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
    sf::Image image = d.getImage();
    std::deque<std::tuple<uint64_t , sf::Image>> replay;
    while (!exit) {
        int event = 0;
        // Was a replay requested?
        if (doDisplay) {
            // Compute all moves
            replay.clear();
            flappyBirdLE.reset(0, Learn::LearningMode::VALIDATION);
            for (auto action = 0; action < params.maxNbActionsPerEval && !flappyBirdLE.isTerminal(); action++) {
                auto vertexList = tee.executeFromRoot(**bestRoot);
                const auto actionID = ((const TPG::TPGAction*)vertexList.back())->getActionID();

                replay.push_back(std::make_tuple(action,  flappyBirdLE.getPicture()));

                flappyBirdLE.doAction(actionID);
                if (flappyBirdLE.isTerminal()){
                    replay.push_back(std::make_tuple(action,  flappyBirdLE.getPicture()));
                }

            }
            // moves computed doDisplay = false <=> replay has been created
            doDisplay = false;
        }

        if (!replay.empty()) {
            frame = std::get<0>(replay.front());
            image = std::get<1>(replay.front());
            replay.pop_front();
       }
        event = d.renderEnv(image, generation, frame);

        switch (event) {
            case 1 :
                std::cout << std::endl << "Display " << ((toggleDisplay) ? "de" : "re") << "activated."
                          << std::endl;
                toggleDisplay = !toggleDisplay;
                break;
            case -1 :
                    exit = true;
                    doDisplay = false;
                    break;
            case 0 :
            default:
                // Nothing to do
                break;
        }
    }
    exit = true;
    printf("\nProgram will terminate at the end of next generation.\n");
    std::cout.flush();
}

int Render::Display::renderEnv(const sf::Image &imageRef, uint64_t generation, uint64_t frame) {

    int returnCode = 0;
    sf::Texture texture;
    sf::Sprite spr;
    if (imageRef.getSize().x > 0 && imageRef.getSize().y > 0) {
        texture.loadFromImage(imageRef);
        spr.setTexture(texture);
        generationText.setString("GEN " + std::to_string(generation));
        frameText.setString("FRAME " + std::to_string(frame));
        window.clear();
        window.draw(spr);
        window.draw(generationText);
        window.draw(frameText);

        window.display();

    }

    // smoothing display by adding delay
    std::this_thread::sleep_for(deltaTime);
    //TODO problème ici en nbThread >= 3 le programme plante Refuse de pool des events => Solved sans le IF
    sf::Event event;
    window.pollEvent(event);
    switch (event.type) {
        case sf::Event::KeyPressed :
            switch (event.key.code) {
                case sf::Keyboard::T :
                    returnCode = 1;
                    break;
                case sf::Keyboard::Q :
                    returnCode = -1;
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
    return returnCode;
}

sf::Image Render::Display::getImage() const {
    return window.capture();
}


