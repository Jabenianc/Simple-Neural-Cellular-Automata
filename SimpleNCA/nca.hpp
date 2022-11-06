
// Short project by Jaben
#ifndef NCA_H
#define NCA_H

// SDL library for rendering
#include <SDL2/SDL.h>
#include <tbb/parallel_for.h>

// Standard output, file reading, string stuff
#include <iostream>
#include <algorithm>
#include <cctype>

// Activation function
#include <functional>
#include <type_traits>

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

namespace util {
    auto to_lowercase(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
}

namespace nca {
    // Functions
    inline constexpr float lerp(float a, float b, float t) noexcept {
        return a + t * (b - a);
    }
    inline constexpr float clamp(float min, float max, float value) noexcept {
        return (value > max)? max : (value < min)? min : value;
    }
    inline constexpr int wrap(int min, int max, int value) noexcept {
        return (value > max)? min : (value < min)? max : value;
    }

    // Quick and lazy system for rng
    float uniform(float min, float max) noexcept {
        if (std::fabs(min - max) == 0) { return 0; }
        std::uniform_real_distribution<float> distr(min, max);
        return distr(global::generator);
    }

    // Struct definitions/Class definitions
    // Rgb class
    class Rgb {
    public:
        explicit constexpr Rgb(uint8_t r, uint8_t g, uint8_t b): r(r), g(g), b(b) {}
        inline constexpr static Rgb interpolate(Rgb const& h, Rgb const& k, float t) noexcept {
            return Rgb(
                lerp(h.r, k.r, t),
                lerp(h.g, k.g, t),
                lerp(h.b, k.b, t)
            );
        }

        // Give each channel a byte
        uint8_t r, g, b;
    };

    // Convolution class
    class Convolution {
    public:
        constexpr Convolution(
            float newa=0.0f, float newb=0.0f, float newc=0.0f,
            float newd=0.0f, float newe=0.0f, float newf=0.0f,
            float newg=0.0f, float newh=0.0f, float newi=0.0f):
            a{newa}, b{newb}, c{newc},
            d{newd}, e{newe}, f{newf},
            g{newg}, h{newh}, i{newi}
        {}

        // Set all values of the matrix
        constexpr void set(
            float newa, float newb, float newc,
            float newd, float newe, float newf,
            float newg, float newh, float newi) noexcept
        {
            a = newa, b = newb, c = newc,
            d = newd, e = newe, f = newf,
            g = newg, h = newh, i = newi;
        }

        // Left side is mapped to left side
        constexpr void set_v_symmetry(
            float topleft,    float topcenter,
            float left,       float center,
            float bottomleft, float bottomcenter) noexcept
        {
            // Left right symmetry
            a = topleft,    b = topcenter,    c = topleft;
            d = left,       e = center,       f = left;
            g = bottomleft, h = bottomcenter, i = bottomleft;
        }

        // Top side is mapped to bottom side
        constexpr void set_h_symmetry(
            float topleft, float topcenter, float topright,
            float left,    float center,    float right) noexcept
        {
            // Top bottom symmetry
            a = topleft, b = topcenter, c = topright;
            d = left,    e = center,    f = right,
            g = topleft, h = topcenter, i = topright;
        }

        // Top left corner mapped to all corners
        // left mapped to right, top mapped to bottom
        constexpr void set_vh_symmetry(
            float corner, float top,
            float left,   float center) noexcept
        {
            // Two lines of symmetry (vertical and horizontal)
            a = corner, b = top,    c = corner;
            d = left,   e = center, f = left,
            g = corner, h = top,    i = corner;
        }

        // Top left corner mapped to corners,
        // sides mapped to sides, center stays center
        constexpr void set_full_symmetry(
            float corner, float side,
                          float center) noexcept
        {
            a = corner, b = side,   c = corner;
            d = side,   e = center, f = side,
            g = corner, h = side,   i = corner;
        }
    public:
        float a{0.0f}, b{0.0f}, c{0.0f},
              d{0.0f}, e{0.0f}, f{0.0f},
              g{0.0f}, h{0.0f}, i{0.0f};

        // Preset convolutions
        static const Convolution WORM;
        static const Convolution WALL;
        static const Convolution SLIME_MOLD;
        static const Convolution STARS;
        static const Convolution MITOSIS;
        static const Convolution WAVES;
    };

    // Preset filters
    constexpr Convolution Convolution::WORM = Convolution(
        0.68, -0.9, 0.68,
        -0.9, -0.66, -0.9,
        0.68, -0.9, 0.68
    );
    constexpr Convolution Convolution::WALL = Convolution(
        0.565, -0.736, 0.565,
        -0.716, 0.627, -0.716,
        0.565, -0.736, 0.565
    );
    constexpr Convolution Convolution::SLIME_MOLD = Convolution(
        0.8, -0.85, 0.8,
        -0.85, -0.2, -0.85,
        0.8, -0.85, 0.8
    );
    constexpr Convolution Convolution::STARS = Convolution(
        0.565, -0.716, 0.565,
        -0.759, 0.627, -0.759,
        0.565, -0.716, 0.565
    );
    constexpr Convolution Convolution::MITOSIS = Convolution(
        -0.939, 0.88, -0.939,
         0.88,  0.4,   0.88,
        -0.939, 0.88, -0.939
    );
    constexpr Convolution Convolution::WAVES = Convolution(
        0.565, -0.716, 0.565,
        -0.716, 0.627, -0.716,
        0.565, -0.716, 0.565
    );


    class Activation {
    public:
        using activation_function = std::function<float(float)>;

        constexpr Activation() = default;
        explicit Activation(activation_function func):
            _func(func)
        {}

        void set_function(activation_function func) {
            _func = func;
        }

        constexpr void clear_function() noexcept {
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
        static const activation_function ABSOLUTE2;
        static const activation_function TANH;
        static const activation_function IGAUSSIAN;
        static const activation_function SPECIALGAUSSIAN;
        static const activation_function WORMGAUSSIAN;
        static const activation_function CELLGAUSSIAN;
    private:
        activation_function _func{};
    };

    // Preset activation functions
    const Activation::activation_function Activation::IDENTITY =
        [](float x) noexcept -> float {
            return x;
        };
    const Activation::activation_function Activation::SIN =
        [](float x) noexcept -> float {
            return std::sin(x);
        };
    const Activation::activation_function Activation::SQUARE =
        [](float x) noexcept -> float {
            return x*x;
        };
    const Activation::activation_function Activation::ABSOLUTE =
        [](float x) noexcept -> float {
            return std::fabs(x);
        };
    const Activation::activation_function Activation::ABSOLUTE2 =
        [](float x) noexcept -> float {
            return std::fabs(1.2f * x);
        };
    const Activation::activation_function Activation::TANH =
        [](float x) noexcept -> float {
            return std::tanh(x);
        };
    const Activation::activation_function Activation::IGAUSSIAN =
        [](float x) noexcept -> float {
            return (-1.0f / std::pow(2.0f, x*x)) + 1.0f;
        };
    const Activation::activation_function Activation::SPECIALGAUSSIAN =
        [](float x) noexcept -> float {
            return (-1.0f / (0.89f*x*x + 1.0f)) + 1.0f;
        };
    const Activation::activation_function Activation::WORMGAUSSIAN =
        [](float x) noexcept -> float {
            return -1.0f/ std::pow(2.0f, (0.6f * std::pow(x, 2.0f))) + 1.0f;
        };
    const Activation::activation_function Activation::CELLGAUSSIAN =
        [](float x) noexcept -> float {
            return -1.0f / (0.9f * std::pow(x, 2.0f) + 1.0f) + 1.0f;
        };

    // Structure to represent an convolution filter and
    // a activation function!
    struct SimulationData {
        SimulationData(Convolution filter, Activation func):
            filter{filter},
            activation{func}
        {};

        Convolution filter{};
        Activation activation{};

        // Some defaulted settings
        static const SimulationData WORMSIM;
        static const SimulationData WALLSIM;
        static const SimulationData SLIMEMOLDSIM;
        static const SimulationData STARSSIM;
        static const SimulationData MITOSISSIM;
        static const SimulationData WAVESSIM;
    };

    const SimulationData SimulationData::WORMSIM = {
        Convolution::WORM, Activation(Activation::WORMGAUSSIAN)
    };

    const SimulationData SimulationData::WALLSIM = {
        Convolution::WALL, Activation(Activation::ABSOLUTE2)
    };

    const SimulationData SimulationData::SLIMEMOLDSIM = {
        Convolution::SLIME_MOLD, Activation(Activation::SPECIALGAUSSIAN)
    };

    const SimulationData SimulationData::STARSSIM = {
        Convolution::STARS, Activation(Activation::ABSOLUTE)
    };

    const SimulationData SimulationData::MITOSISSIM = {
        Convolution::MITOSIS, Activation(Activation::CELLGAUSSIAN)
    };

    const SimulationData SimulationData::WAVESSIM = {
        Convolution::WAVES, Activation(Activation::ABSOLUTE2)
    };

    class WorldGrid {
    public:
        using Grid2D = std::vector<std::vector<float>>;
        WorldGrid(int width, int height):
            ROWS(width), COLUMNS(height),
            lowerbound_color(0x00, 0x00, 0x00),
            upperbound_color(0x00, 0x00, 0x00),
            activation{},
            filter{},
            _p_newgrid(new Grid2D()),
            _p_grid(new Grid2D())
        {
            // Fill the grid with random cells.
            for (int i = 0; i < ROWS; i++) {
                _p_grid->emplace_back(std::vector<float>{});

                // Push back random floats
                for (int j = 0; j < COLUMNS; j++) {
                    _p_grid->at(i).push_back(uniform(0.0f, 1.0f));
                }
            }
            // Fill the new grid with empty cells
            for (int i = 0; i < ROWS; i++) {
                _p_newgrid->push_back(std::vector<float>{});

                // Push back blank floats
                for (int j = 0; j < COLUMNS; j++) {
                    _p_newgrid->at(i).push_back(0.0f);
                }
            }
        }

        // Not implementing copies for this.
        WorldGrid(const WorldGrid&) = delete;
        WorldGrid& operator=(const WorldGrid&) = delete;

        ~WorldGrid() {
            delete _p_grid;
            delete _p_newgrid;
        }
    public:
        // Set world attributes via simulation data struct
        void add_data(SimulationData const& data) noexcept {
            filter = data.filter;
            activation = data.activation;
        }

        template<
            typename T,
            typename = std::enable_if_t<std::is_base_of_v<Convolution, std::decay_t<T>>>
        >
        void add_convolution(T&& new_filter) noexcept {
            filter = std::forward<T>(new_filter);
        }

        constexpr void add_activation(Activation const& new_activation) noexcept {
            activation = new_activation;
        }

        constexpr void set_lowerbound_color(Rgb const& color) noexcept {
            lowerbound_color = color;
        }

        constexpr void set_upperbound_color(Rgb const& color) noexcept {
            upperbound_color = color;
        }

        constexpr Rgb color_at(int x, int y) const noexcept(noexcept(_p_grid->at(x).at(y)))
        {
            return Rgb::interpolate(
                lowerbound_color,
                upperbound_color,
                _p_grid->at(x).at(y)
            );
        }

        constexpr int get_rows() const noexcept { return ROWS; }
        constexpr int get_columns() const noexcept { return COLUMNS; }

        // Simulation function.
        void simulate() {

        // Apply Convolution
        tbb::parallel_for(tbb::blocked_range<int>(0, ROWS),
            [&](tbb::blocked_range<int> r) {
                for (auto x = r.begin(); x < r.end(); x++) {
                    for (int y{0}; y < COLUMNS; y++) {
                        // xplus and xminus coordinates, wrapped to the plane
                        const float xp = wrap(0, ROWS-1, x+1), xm = wrap(0, ROWS-1, x-1);
                        // yminus and yplus coordinates, wrapped to the plane
                        const float yp = wrap(0, COLUMNS-1, y+1), ym = wrap(0, COLUMNS-1, y-1);

                        // Filter step
                        (*_p_newgrid)[x][y] =
                               // Row 1 (a - c)
                               (*_p_grid)[xm][yp] * filter.a +
                               (*_p_grid)[x][yp]  * filter.b +
                               (*_p_grid)[xp][yp] * filter.c +

                               // Row 2 (d - f)
                               (*_p_grid)[xm][y]  * filter.d +
                               (*_p_grid)[x][y]   * filter.e +
                               (*_p_grid)[xp][y]  * filter.f +

                               // Row 3 (g - i)
                               (*_p_grid)[xm][ym] * filter.g +
                               (*_p_grid)[x][ym]  * filter.h +
                               (*_p_grid)[xp][ym] * filter.i;
                    }
                }
            });

            // Apply activation
            for (int x{0}; x < ROWS; x++) {
                for (int y{0}; y < COLUMNS; y++) {
                    (*_p_newgrid)[x][y] = clamp(0.0f, 1.0f,
                        activation.activate((*_p_newgrid)[x][y]));
                }
            }

            // Set _p_grid pointer to _p_newgrid
            std::swap(_p_grid, _p_newgrid);
        }
    private:
        const int ROWS;
        const int COLUMNS;
        Rgb lowerbound_color;
        Rgb upperbound_color;
        Activation activation;
        Convolution filter;
        Grid2D* _p_newgrid;
        Grid2D* _p_grid;
    };
}


#endif /* end of include guard: NCA_H */
