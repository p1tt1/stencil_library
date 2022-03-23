
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
#include <cassert>
#include <iostream>
#include <omp.h>
#include "vcl/vectorclass.h"

using VALUE_TYPE = double;
using VEC_TYPE = Vec4d;

constexpr std::size_t OBJ_COLS =   1500;
constexpr std::size_t OBJ_ROWS =   1500;
constexpr std::size_t OBJ_LEVELS = 1500;

constexpr VALUE_TYPE C = 1.0;
constexpr VALUE_TYPE H = 1.0;
constexpr VALUE_TYPE TAU = 1.0;

constexpr std::size_t RUNS = 1;

constexpr std::size_t RND_MAX = 100;
constexpr std::size_t RND_SEED = 1;

constexpr std::size_t ITER_SOLVER_MAX = 5;
constexpr std::size_t ITER_STEP_CALC_MAX = 5;

constexpr VALUE_TYPE EPSILON_OPERATOR = 1e-100;
constexpr VALUE_TYPE EPSILON_SOLVER = 1e-16;
constexpr VALUE_TYPE EPSILON_STEP_CALC = 1e-16;

#include "c_nonlinear_stencil_precalc.hpp"
#include "c_state_function_mul2.hpp"
#include "c_cg.hpp"
#include "c_timestep_calculator.hpp"

template <template<template<typename VecType> class StateFunction, typename ValueType, typename VecType> class OperatorType, template<typename VecType> class StateFunction, typename ValueType, typename VecType>
void routine(std::size_t p_objCols,
             std::size_t p_objRows,
             std::size_t p_objLevels,
             std::size_t p_runs
)
{
    std::size_t l_objSize2d = p_objCols * p_objRows;
    std::size_t l_objCells = l_objSize2d * p_objLevels;

    std::size_t l_iterStepCalc;

    //
    // NOTE: wird initial als x_0, y und state benutzt...
    //
    ValueType * l_b_raw = new ValueType[l_objCells+2*l_objSize2d];
    ValueType * l_b = &(l_b_raw[l_objSize2d]);

    ValueType * l_x = new ValueType[l_objCells];

    //
    // NOTE: first touch initialization
    //
    #pragma omp parallel
    {
        #pragma omp for
        for (std::size_t i = 0; i < l_objCells; ++i)
        {
            l_x[i] = 0;

            l_b[i] = i;
        }

        #pragma omp for
        for (std::size_t i = 0; i < l_objSize2d; ++i)
        {
            l_b_raw[i] = 0;
            l_b_raw[l_objCells+l_objSize2d+i] = 0;
        }
    }

    OperatorType<StateFunction,ValueType,VecType> l_Op(
        p_objCols,
        p_objRows,
        p_objLevels,
        l_b,
        H,
        TAU,
        EPSILON_OPERATOR
    );

    C_CG<ValueType> l_solver;
    C_TimestepCalculator<ValueType> l_stepCalc;

    double l_tStartApply = omp_get_wtime();

    #pragma omp parallel
    {
        LIKWID_MARKER_START("Apply");
    }

    for (std::size_t i = 0; i < p_runs; ++i)
    {
        l_iterStepCalc = l_stepCalc(
            l_objCells,
            l_solver,
            l_Op,
            l_b,
            l_x,
            EPSILON_SOLVER,
            EPSILON_STEP_CALC,
            ITER_SOLVER_MAX,
            ITER_STEP_CALC_MAX,
            l_objSize2d);
    }

    #pragma omp parallel
    {
        LIKWID_MARKER_STOP("Apply");
    }

    double l_tEndApply = omp_get_wtime();
    double l_tApply = l_tEndApply - l_tStartApply;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "OBJ_COLS_IMPL," << p_objCols << std::endl;
    std::cout << "OBJ_ROWS_IMPL," << p_objRows << std::endl;
    std::cout << "OBJ_LEVELS_IMPL," << p_objLevels << std::endl;
    std::cout << "OBJ_CELLS_IMPL," << l_objCells << std::endl;
    std::cout << "IMPL_ID_IMPL," << "cg_nonlinear_precalc" << std::endl;
    std::cout << "FUNC_ID_IMPL," << "mul2" << std::endl;
    std::cout << "RUNTIME_APPLY_IMPL," << l_tApply << std::endl;
    std::cout << "ITER_STEP_CALC_IMPL," << l_iterStepCalc << std::endl;
    std::cout << "ITER_SOLVER_MAX_IMPL," << ITER_SOLVER_MAX << std::endl;
    std::cout << "ITER_STEP_CALC_MAX_IMPL," << ITER_STEP_CALC_MAX << std::endl;
    std::cout << "EPSILON_OPERATOR_IMPL," << EPSILON_OPERATOR << std::endl;
    std::cout << "EPSILON_SOLVER_IMPL," << EPSILON_SOLVER << std::endl;
    std::cout << "EPSILON_STEP_CALC_IMPL," << EPSILON_STEP_CALC << std::endl;

    delete [] l_b_raw;
    delete [] l_x;
}

int main(int argc, char *argv[])
{
    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;
    #pragma omp parallel
    {
        LIKWID_MARKER_REGISTER("Apply");
    }

    srand(RND_SEED);
    std::cout << "argc: " << argc << std::endl;
    std::cout << "argv: [";
    for (std::size_t i = 0; i < argc; ++i)
    {
        std::cout << argv[i];
        if (i < argc - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    if(argc != 1 && argc != 4 && argc != 5)
    {
        std::cout << "INVALID INPUT: argc must be 1 or 4 or 5" << std::endl;
        return 1;
    }

    std::size_t l_objCols = OBJ_COLS;
    std::size_t l_objRows = OBJ_ROWS;
    std::size_t l_objLevels = OBJ_LEVELS;
    std::size_t l_runs = RUNS;

    if (argc >= 4)
    {
        l_objCols =   atoi(argv[1]);
        l_objRows =   atoi(argv[2]);
        l_objLevels = atoi(argv[3]);
    }
    if (argc == 5)
    {
        l_runs =   atoi(argv[4]);
    }
    double l_tStartRoutine = omp_get_wtime();
    routine<CNonlinearStencilPrecalc, CStateFunctionMul2, VALUE_TYPE, VEC_TYPE>(
        l_objCols,
        l_objRows,
        l_objLevels,
        l_runs
    );
    double l_tEndRoutine = omp_get_wtime();
    double l_tRoutine = l_tEndRoutine - l_tStartRoutine;

    //
    // NOTE: output is parsed by bench script
    //
    std::cout << "RUNTIME_OVERALL_IMPL," << l_tRoutine << std::endl;

    LIKWID_MARKER_CLOSE;
    return 0;
}
