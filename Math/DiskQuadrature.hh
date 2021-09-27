/// \file DiskQuadrature.hh Quadrature integration rules for disk
// Abramowitz and Stegun 25.4.61 (p. 891--)

// x, y, weight, for disk radius = 0.5

/// single point at center
const double diskquad_1[][3] = { {0.,0.,1.} };

/// four-point disc integrator
const double diskquad_4[][3] = {
    { .25,  .25, .25},
    { .25, -.25, .25},
    {-.25,  .25, .25},
    {-.25, -.25, .25} };

/// nine-point disc integrator
const double diskquad_9[][3] = {
    {  0,    0,  1/6.},
    { .25,  .25, 1/6.},
    { .25, -.25, 1/6.},
    {-.25,  .25, 1/6.},
    {-.25, -.25, 1/6.},
    { .5,    0,  1/24.},
    {-.5,    0,  1/24.},
    {  0,   .5,  1/24.},
    {  0,  -.5,  1/24.} };
