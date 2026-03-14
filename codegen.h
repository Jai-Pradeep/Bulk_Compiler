#pragma once
#include "regalloc.h"

// Generate x86-64 assembly from register-allocated IR
void generateAssembly(const std::string& filename, const RegisterAllocation& alloc);