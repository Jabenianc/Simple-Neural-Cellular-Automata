
// Short project by Jaben
#ifndef NCA_H
#define NCA_H

// SDL library for rendering
#include <SDL2/SDL.h>

// Standard output
#include <iostream>

// Activation function
#include <functional>

// Colors, math, matrices, etc.
#include <cstdint>
#include <vector>
#include <cmath>
#include <random>

#define unless(expression) if(!(expression))

// Global random number generator
namespace nca::global {
    std::random_device rd{};
    std::mt19937 generator = std::mt19937(rd());
}

namespace nca {
    // Functions
    inline constexpr float lerp(float a, float b, float t) noexcept {
        return a * (1.0f - t) + (b * t);
    }
    inline constexpr float clamp(float min, float max, float value) {
        return (value > max)? max : (value < min)? min : value;
    }
    inline constexpr int wrap(int min, int max, int value) {
        return (value > max)? min : (value < min)? max : value;
    }

    // Quick and lazy system for rng.
    float uniform(float min, float max) {
        if (std::fabs(min - max) == 0) { return 0; }
        std::uniform_real_distribution<float> distr(min, max);
        return distr(global::generator);
    }

    // Struct definitions/Class definitions
    class Rgb {
    public:
        Rgb(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
        static Rgb interpolate(Rgb const& h, Rgb const& k, float t) noexcept {
            return Rgb(
                lerp(h.r, k.r, t),
                lerp(h.g, k.g, t),
                lerp(h.b, k.b, t)
            );
        }
        uint8_t r, g, b;
    };

    class Convolution {
    public:
        // Constructor
        Convolution(): _matrix(new float*[3]) {
            for (int i = 0; i < 3; i++) {
                _matrix[i] = new float[3];
                for (int j = 0; j < 3; j++) {
                    _matrix[i][j] = 0.0f;
                }
            }
        }

        // Destructor
        ~Convolution() {
            for (int i = 0; i < 3; i++) {
                delete[] _matrix[i];
            }
            delete[] _matrix;
        }

        // Copy ctor
        Convolution(Convolution const& other) {
            // First initialize
            for (int i = 0; i < 3; i++) {
                other._matrix[i] = new float[3];
            }
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    other._matrix[i][j] = _matrix[i][j];
                }
            }
        }

        // Overloaded assignment
        Convolution& operator=(Convolution const& other) {
            unless (this == &other) {
                // Set the values of this
                // matrix to the other ones.
                // No need for deletion of resources.
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        _matrix[i][j] = other._matrix[i][j];
                    }
                }
            }
            return *this;
        }
    public:
        double at(unsigned int x, unsigned int y) const noexcept {
            return (x < 3 && y < 3)? _matrix[x][y] : 0.0f;
        }
        void set_matrix(float a, float b, float c,
                        float d, float e, float f,
                        float g, float h, float i)
        {
            _matrix[0][0] = a; _matrix[1][0] = b; _matrix[2][0] = c;
            _matrix[0][1] = d; _matrix[1][1] = e; _matrix[2][1] = f;
            _matrix[0][2] = g; _matrix[1][2] = h; _matrix[2][2] = i;
        }
    private:
        float** _matrix;
    };

    class Activation {
    public:
        using activation_function = std::function<float(float)>;
        Activation() = default;
        void set_function(activation_function func) {
            _func = func;
        }
        void clear_function() {
            _func = {};
        }
        constexpr float activate(float x) const {
            return (_func)? _func(x) : x;
        }

        // Some activation functions built in
        static const activation_function IDENTITY;
        static const activation_function SIN;
        static const activation_function SQUARE;
        static const activation_function ABSOLUTE;
        static const activation_function TANH;
        static const activation_function IGAUSSIAN;
        static const activation_function SPECIALGAUSSIAN;
    private:
        activation_function _func {};
    };

    // Preset activation functions
    const Activation::activation_function Activation::IDENTITY =
        [](float x) noexcept {
            return x;
        };
    const Activation::activation_function Activation::SIN =
        [](float x) noexcept {
            return std::sin(x);
        };
    const Activation::activation_function Activation::SQUARE =
        [](float x) noexcept {
            return x*x;
        };
    const Activation::activation_function Activation::ABSOLUTE =
        [](float x) noexcept {
            return std::fabs(x);
        };
    const Activation::activation_function Activation::TANH =
        [](float x) noexcept {
            return std::tanh(x);
        };
    const Activation::activation_function Activation::IGAUSSIAN =
        [](float x) noexcept {
            return (-1.0f / std::pow(2.0f, x*x)) + 1.0f;
        };
    const Activation::activation_function Activation::SPECIALGAUSSIAN =
        [](float x) noexcept {
            return (-1.0f / (0.89*x*x + 1.0f)) + 1.0f;
        };


    class WorldGrid {
    public:
        using Grid2D = std::vector<std::vector<float>>;
        WorldGrid(int width, int height):
            ROWS(width), COLUMNS(height),
            _p_lowerbound_color(nullptr),
            _p_upperbound_color(nullptr),
            _p_activation_func(nullptr),
            _p_filter(nullptr),
            _p_newgrid(new Grid2D()),
            _p_grid(new Grid2D())
        {
            // Fill the grid with random cells.
            for (int i = 0; i < ROWS; i++) {
                _p_grid->push_back(std::vector<float>{});
                for (int j = 0; j < COLUMNS; j++) {
                    _p_grid->at(i).push_back(uniform(0.0f, 1.0f));
                }
            }
            // Fill the new grid with empty cells
            for (int i = 0; i < ROWS; i++) {
                _p_newgrid->push_back(std::vector<float>{});
                for (int j = 0; j < COLUMNS; j++) {
                    _p_newgrid->at(i).push_back(0.0f);
                }
            }
        }
        // Not implementing copies for this.

        ~WorldGrid() {
            // Don't delete other resources we aren't managing ourselves
            delete _p_lowerbound_color;
            delete _p_upperbound_color;
            delete _p_grid;
        }
    public:
        void add_convolution(Convolution* p_filter) {
            _p_filter = p_filter;
        }

        void add_activation(Activation* p_activation) {
            _p_activation_func = p_activation;
        }

        void set_lowerbound_color(Rgb const& color) {
            if (_p_lowerbound_color != nullptr) {
                delete _p_lowerbound_color;
                _p_lowerbound_color = nullptr;
            }
            _p_lowerbound_color = new Rgb(color);
        }

        void set_upperbound_color(Rgb const& color) {
            if (_p_upperbound_color != nullptr) {
                delete _p_upperbound_color;
                _p_upperbound_color = nullptr;
            }
            _p_upperbound_color = new Rgb(color);
        }

        Rgb color_at(int x, int y) const {
            return Rgb::interpolate(
                *_p_lowerbound_color,
                *_p_upperbound_color,
                _p_grid->at(x).at(y)
            );
        }
        constexpr int get_rows() const noexcept { return ROWS; }
        constexpr int get_columns() const noexcept { return COLUMNS; }

        // Simulation function.
        void simulate() {
            // Apply Convolution
            for (int x = 0; x < ROWS; x++) {
                for (int y = 0; y < COLUMNS; y++) {
                    float sum(0.0f);

                    // Alterned coordinates with wrapping
                    float xp = wrap(0, ROWS-1, x+1), xm = wrap(0, ROWS-1, x-1);
                    float yp = wrap(0, COLUMNS-1, y+1), ym = wrap(0, COLUMNS-1, y-1);

                    // Center
                    sum += _p_grid->at(x).at(y) * _p_filter->at(1, 1);
                    // Top bottom
                    sum += _p_grid->at(x).at(yp) * _p_filter->at(0, 1);
                    sum += _p_grid->at(x).at(ym) * _p_filter->at(1, 2);
                    // Left Right
                    sum += _p_grid->at(xp).at(y) * _p_filter->at(2, 1);
                    sum += _p_grid->at(xm).at(y) * _p_filter->at(0, 1);
                    // Top corners
                    sum += _p_grid->at(xp).at(yp) * _p_filter->at(2, 0);
                    sum += _p_grid->at(xm).at(yp) * _p_filter->at(0, 0);
                    // Bottom corners
                    sum += _p_grid->at(xp).at(ym) * _p_filter->at(2, 2);
                    sum += _p_grid->at(xm).at(ym) * _p_filter->at(0, 2);

                    // Apply it to the new grid's sum
                    _p_newgrid->at(x).at(y) = sum;
                }
            }

            // Apply activation
            for (int x = 0; x < ROWS; x++) {
                for (int y = 0; y < COLUMNS; y++) {
                    float z = clamp(0.0f, 1.0f,
                        _p_activation_func->activate(_p_newgrid->at(x).at(y)));
                    _p_newgrid->at(x).at(y) = z;
                }
            }

            // Set _p_grid pointer to _p_newgrid
            std::swap(_p_grid, _p_newgrid);
        }
    private:
        const int ROWS;
        const int COLUMNS;
        Rgb* _p_lowerbound_color;
        Rgb* _p_upperbound_color;
        Activation* _p_activation_func;
        Convolution* _p_filter;
        Grid2D* _p_newgrid;
        Grid2D* _p_grid;
    };
}


#endif /* end of include guard: NCA_H */
