
// This block enables to compile the code with and without the likwid header in place
#ifdef LIKWID_PERFMON
#include <likwid-marker.h>
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_SWITCH
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#define LIKWID_MARKER_GET(regionTag, nevents, events, time, count)
#endif

#include <cstddef>

using VALUE_TYPE = double;

constexpr std::size_t FACTOR = 1000;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RUNS = 1;

#include "c_linear_stencil.hpp"

template <template<typename ValueType> class OperatorType, typename ValueType>
void routine()
{

    std::size_t l_sCols = FACTOR;
    std::size_t l_sRows = FACTOR;
    std::size_t l_sLevels = FACTOR;

    std::size_t l_sCells = l_sCols * l_sRows * l_sLevels;

    std::size_t l_iter;
    ValueType * l_x = new ValueType[l_sCells];
    ValueType * l_y = new ValueType[l_sCells];

    for (std::size_t i = 0; i < l_sCells; ++i)
    {
        l_x[i] = i;
    }

    OperatorType<ValueType> l_Op(
        l_sCols,
        l_sRows,
        l_sLevels,
        C,
        H,
        TAU
    );

    LIKWID_MARKER_START("Apply");
    for (std::size_t i = 0; i < RUNS; ++i)
    {
        l_Op.apply(l_x,l_y);
    }
    LIKWID_MARKER_STOP("Apply");

    delete [] l_x;
    delete [] l_y;
}

int main()
{
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    LIKWID_MARKER_REGISTER("Apply");

    routine<CLinearStencil, VALUE_TYPE>();

    LIKWID_MARKER_CLOSE;
    return 0;
}
