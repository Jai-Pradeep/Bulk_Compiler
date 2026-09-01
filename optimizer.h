#pragma once
#include "ir.h"

// ── Optimization levels ───────────────────────────────────────────────────────
//
//  -O0  (level 0)  no optimization at all — raw IR as generated
//
//  -O1  (level 1)  one round of:
//                    - constant folding
//                    - copy propagation
//                    - common subexpression elimination
//                   - loop-invariant code motion
//                    - dead code elimination
//
//  -O2  (level 2)  same passes but repeated until fully stable
//                  (folding can expose new propagation opportunities,
//                   propagation can expose new constants, etc.)
//
void optimizeIR(int level = 1);
