// Short project by Jaben
// Small simulation of neural cellular automata
#include "nca.hpp"

// Constants
constexpr int SCREEN_HEIGHT = 420;
constexpr int SCREEN_WIDTH = 750;
constexpr int FAIL = 0;
constexpr int NOEVENT = 0;

// Global SDL structs
SDL_Window* gwindow = nullptr;
SDL_Renderer* grenderer = nullptr;

// Main function
int main(void) {
    bool success(true);
    if (SDL_Init(SDL_INIT_VIDEO) < FAIL) {
        std::cout << "SDL could not initialize! "
            << SDL_GetError() << "\n";
        success = false;
    } else {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cout << "Warning: Linear texture filtering not enabled!!\n";
        }
        gwindow = SDL_CreateWindow("Neural Cellular Automata",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gwindow == nullptr) {
            std::cout << "Couldn't create window!! " << SDL_GetError() << "\n";
            success = false;
        } else {
            grenderer = SDL_CreateRenderer(gwindow, -1, SDL_RENDERER_ACCELERATED);
            if (grenderer == nullptr) {
                std::cout << "Renderer couldn't be created!! " << SDL_GetError() << "\n";
                success = false;
            }
            else {
                // Render draw color
                SDL_SetRenderDrawColor(grenderer, 0, 0, 0, 0);
            }
        }
    }

    if (success) {
        // Main loop
        bool simulation_active{true};

        // World sim
        auto world = new nca::WorldGrid(SCREEN_WIDTH, SCREEN_HEIGHT);

        // World setup.
        std::string input{"worm"};
        std::cout << "Please enter simulation type: worm, star, cell, wall, wave, slime\n";
        std::cout << "Type: ";
        std::cin >> input;
        input = util::to_lowercase(input);

        // Should use a hashmap optimally
        if (input == "worm") {
            world->add_data(nca::SimulationData::WORMSIM);
        }
        else if (input == "star") {
            world->add_data(nca::SimulationData::STARSSIM);
        }
        else if (input == "cell") {
            world->add_data(nca::SimulationData::MITOSISSIM);
        }
        else if (input == "wall") {
            world->add_data(nca::SimulationData::WALLSIM);
        }
        else if (input == "wave") {
            world->add_data(nca::SimulationData::WAVESSIM);
        }
        else if (input == "slime") {
            world->add_data(nca::SimulationData::SLIMEMOLDSIM);
        }
        else {
            std::cout << "Defaulting to worm simulation\n";
            world->add_data(nca::SimulationData::WORMSIM);
        }

        world->set_lowerbound_color(nca::Rgb(231, 51, 35));
        world->set_upperbound_color(nca::Rgb(135, 173, 188));

        // Event struct
        SDL_Event e{};
        int frame_index = -1;

        while (simulation_active) {
            // Handle events on queue
	        while (SDL_PollEvent(&e) != NOEVENT) {
			    if (e.type == SDL_QUIT) {
					simulation_active = false;
				}
			}

            // Flush screen.
            SDL_SetRenderDrawColor(grenderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(grenderer);

            // Simulation step.
            world->simulate();

            // Render everything
            for (int cellx = 0; cellx < world->get_rows(); cellx++) {
                for (int celly = 0; celly < world->get_columns(); celly++) {
                    const auto&& color = world->color_at(cellx, celly);
                    SDL_SetRenderDrawColor(grenderer, color.r, color.g, color.b, 0xFF);
                    SDL_RenderDrawPoint(grenderer, cellx, celly);
                }
            }

            frame_index *= -1;

             // PLEASE PLEASE PLEASE BY ALL MEANS
             // DO NOT TURN THIS OF THIS IS A FRAME SKIP GUARD FOR
             // MANY PULSING BEHAVIOURS THAN CAN CAUSE
             // BRIGHT FLASHING LIGHTS
            if (frame_index == 1) {
                // Update screen.
    			SDL_RenderPresent(grenderer);
            }
        }
        // Deallocate resources.
        delete world; world = nullptr;
    }

    // Deallocate resources
    SDL_DestroyRenderer(grenderer);
    SDL_DestroyWindow(gwindow);
    grenderer = nullptr, gwindow = nullptr;

    // Quit Sdl
    SDL_Quit();

    // Program end
    return 0;
}
