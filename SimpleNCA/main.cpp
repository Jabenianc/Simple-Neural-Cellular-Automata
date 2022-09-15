// Short project by Jaben
#include "nca.hpp"

// Constants
constexpr int SCREEN_HEIGHT = 600;
constexpr int SCREEN_WIDTH = 1080;
constexpr int FAIL = 0;
constexpr int NOEVENT = 0;
constexpr int SOLID = 0xFF;
constexpr int BLACK[4] = {0x00, 0x00, 0x00, SOLID};

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
                SDL_SetRenderDrawColor(grenderer,
                    BLACK[0], BLACK[1],
                    BLACK[2], BLACK[3]);
            }
        }
    }

    if (success) {
        // Main loop
        bool simulation_active(true);
        auto world = new nca::WorldGrid(SCREEN_WIDTH, SCREEN_HEIGHT);

        // Convolution matrix
        auto convolution = new nca::Convolution();
        convolution->set_matrix(
            0.80, -0.85,  0.80,
           -0.85, -0.20, -0.85,
            0.80, -0.85,  0.80
        );


        // Activation function
        auto activation = new nca::Activation();
        activation->set_function(nca::Activation::SPECIALGAUSSIAN);

        // World setup.
        world->add_convolution(convolution);
        world->add_activation(activation);
        world->set_lowerbound_color(nca::Rgb(0, 0, 0));
        world->set_upperbound_color(nca::Rgb(0, 255, 0));

        // Event struct
        SDL_Event e{};
        int frame_index = -1;
        while (simulation_active) {
            // Handle events on queue
	        while(SDL_PollEvent(&e) != NOEVENT) {
			    if( e.type == SDL_QUIT ) {
					simulation_active = false;
				}
			}
            frame_index *= -1;
            // Flush screen.
            SDL_SetRenderDrawColor(grenderer,
                BLACK[0], BLACK[1],
                BLACK[2], BLACK[3]);
            SDL_RenderClear(grenderer);

            // Simulation step.
            world->simulate();

            for (int cellx = 0; cellx < world->get_rows(); cellx++) {
                for (int celly = 0; celly < world->get_columns(); celly++) {
                    const auto color = world->color_at(cellx, celly);
                    SDL_SetRenderDrawColor(grenderer, color.r, color.g, color.b, SOLID);
                    SDL_RenderDrawPoint(grenderer, cellx, celly);
                }
            }

            if (frame_index == 1) { // Skip even frames
                // Update screen.
    			SDL_RenderPresent(grenderer);
            }
        }
        // Deallocate resources.
        delete world; world = nullptr;
        delete activation; activation = nullptr;
        delete convolution; convolution = nullptr;
    }

    // Deallocate resources
    SDL_DestroyRenderer(grenderer);
    SDL_DestroyWindow(gwindow);
    grenderer = nullptr, gwindow = nullptr;
    SDL_Quit();
    return 0;
}
