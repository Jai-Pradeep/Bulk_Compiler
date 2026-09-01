#include "codegen.h"
#include "ir.h"
#include "symtab.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

// ── C type mapping ────────────────────────────────────────────────────────────
static std::string cType(IRType t) {
    switch(t) {
        case IRType::INT32:  return "int";
        case IRType::INT64:  return "long long";
        case IRType::INT128: return "__int128";
        case IRType::FLOAT:  return "float";
        case IRType::CHAR:   return "char";
        case IRType::BOOL:   return "int";
        case IRType::VOID:   return "void";
        default:             return "int";
    }
}

static std::string printFmt(IRType t) {
    switch(t) {
        case IRType::INT64:  return "%lld";
        case IRType::FLOAT:  return "%f";
        case IRType::CHAR:   return "%c";
        default:             return "%d";
    }
}
static std::string scanFmt(IRType t) {
    switch(t) {
        case IRType::INT64: return "%lld";
        case IRType::FLOAT: return "%f";
        case IRType::CHAR:  return " %c";
        default:            return "%d";
    }
}

static bool isTemp(const std::string& s) {
    if(s.size()<2||s[0]!='t') return false;
    for(size_t i=1;i<s.size();++i) if(!std::isdigit(s[i])) return false;
    return true;
}
static bool isLiteral(const std::string& s) {
    if(s.empty()) return false;
    size_t st=(s[0]=='-')?1:0;
    if(st==s.size()) return false;
    for(size_t i=st;i<s.size();++i) if(!std::isdigit(s[i])) return false;
    return true;
}
static bool isParallelComment(const std::string& s){return s.find("PARALLEL LOOP")!=std::string::npos;}
static bool isLargeLoop(const std::string& b){if(!isLiteral(b)) return false; try{return std::stoi(b)>10000;}catch(...){return false;}}

static std::unordered_map<std::string,IRType> collectTemps(size_t start, size_t end) {
    std::unordered_map<std::string,IRType> temps;
    for(size_t i=start;i<end&&i<ir.size();++i){
        auto& ins=ir[i];
        auto rec=[&](const std::string& n,IRType t){
            if(isTemp(n)&&!temps.count(n)) temps[n]=(t!=IRType::UNKNOWN)?t:IRType::INT32;
        };
        rec(ins.result,ins.type); rec(ins.arg1,ins.type); rec(ins.arg2,ins.type);
    }
    return temps;
}

struct ParallelLoop {
    bool found=false;
    size_t commentIdx,initIdx,lstartIdx,boundIdx,ifzeroIdx,bodyStart,bodyEnd,gotoIdx,lendIdx;
    std::string idxVar,bound,Lstart,Lend,Lstep;
};

static ParallelLoop detectParallelLoop(size_t pos) {
    ParallelLoop pl;
    size_t i=pos;
    while(i<ir.size()&&ir[i].op=="comment") ++i;
    if(i>=ir.size()||ir[i].op!="="||ir[i].arg1!="0") return pl;
    pl.initIdx=i; pl.idxVar=ir[i].result; ++i;
    if(i>=ir.size()||ir[i].op!="label") return pl;
    pl.lstartIdx=i; pl.Lstart=ir[i].arg1; ++i;
    if(i>=ir.size()) return pl;
    pl.boundIdx=i; pl.bound=ir[i].arg2;
    std::string tBound=ir[i].result; ++i;
    if(i>=ir.size()||ir[i].op!="ifzero_goto"||ir[i].arg1!=tBound) return pl;
    pl.ifzeroIdx=i; pl.Lend=ir[i].arg2; ++i;
    pl.bodyStart=i;

    while(i<ir.size()){
        if(ir[i].op=="+"&&ir[i].arg1==pl.idxVar&&ir[i].arg2=="1") break;
        ++i;
    }
    if(i>=ir.size()) return pl;

    pl.bodyEnd=i;
    size_t j=i;
    while(j>pl.bodyStart && ir[j-1].op=="label") --j;
    pl.bodyEnd=j;
    for(size_t k=j;k<i;++k){
        if(ir[k].op=="label") pl.Lstep=ir[k].arg1;
    }

    std::string tStep=ir[i].result; ++i;
    if(i>=ir.size()||ir[i].op!="="||ir[i].arg1!=tStep) return pl;
    ++i;
    if(i>=ir.size()||ir[i].op!="goto"||ir[i].arg1!=pl.Lstart) return pl;
    pl.gotoIdx=i; ++i;
    if(i>=ir.size()||ir[i].op!="label"||ir[i].arg1!=pl.Lend) return pl;
    pl.lendIdx=i; pl.found=true; return pl;
}

// Forward declaration — emitC is defined after emitCUDAKernel but called from it.
static std::string emitC(const IRInstruction& ins, const std::string& ind);

// ── CUDA kernel emission ──────────────────────────────────────────────────────
//
// Analyses the body of the detected parallel loop to figure out which
// arrays are read-only (inputs) and which are written (outputs), then
// emits a complete, self-contained .cu file: kernel + host scaffolding.
//
// Strategy for IR-driven body emission:
//   • Any array indexed by idxVar and assigned          → output array
//   • Any array indexed by idxVar that is only read     → input array
//   • Scalar temporaries are declared as local vars inside the kernel.
//
// For the common case of a simple element-wise loop the emitted kernel
// is directly correct.  More complex bodies fall back to a line-by-line
// transcription of the IR with the same emitC() helper used by the C path.
//
static void emitCUDAKernel(const ParallelLoop& pl,
                           const std::string& cuFile,
                           const std::string& exeName)
{
    // ── 1. Scan the body to discover arrays used ──────────────────────────
    //
    // We look for IR patterns like:
    //   t_idx  = arr + idxVar          (array base + index → address)
    //   result = *t_idx                (load)
    //   *t_idx = value                 (store — represented as "store" op or "=")
    //
    // In practice BulkCompiler typically emits array accesses as:
    //   t1 = arr[i]   →  t1 = *(arr + i)   (load from array+offset)
    //   arr[i] = v    →  *(arr + i) = v     (store)
    //
    // We collect unique array names and classify them.

    struct ArrayInfo {
        std::string name;
        IRType      type = IRType::INT32;
        bool        written = false;   // true → output
        bool        read    = false;   // true → input (or both)
    };
    std::unordered_map<std::string, ArrayInfo> arrays;

    // Helper: record an array use
    auto markArray = [&](const std::string& name, IRType t, bool write) {
        auto& a = arrays[name];
        a.name = name;
        if(t != IRType::UNKNOWN) a.type = t;
        if(write) a.written = true;
        else      a.read    = true;
    };

    for(size_t i=pl.bodyStart; i<pl.bodyEnd && i<ir.size(); ++i){
        auto& ins = ir[i];
        // Array element load:  result = arr + idxVar  then deref
        if((ins.op=="+"||ins.op=="add") && ins.arg2==pl.idxVar)
            markArray(ins.arg1, ins.type, false);
        // Array element store: result written via temp that holds arr+idx
        if(ins.op=="store" || (ins.op=="=" && !ins.arg2.empty()))
            markArray(ins.result, ins.type, true);
        // Direct array[i] = ...  style (result contains array name)
        if(ins.op=="=" && symtab.exists(ins.result)){
            const auto sym = symtab.get(ins.result);
            if(sym.isArray) markArray(ins.result, sym.irType, true);
        }
        if(ins.op=="=" && symtab.exists(ins.arg1)){
            const auto sym = symtab.get(ins.arg1);
            if(sym.isArray) markArray(ins.arg1, sym.irType, false);
        }
    }

    // If discovery found nothing (simple scalar body), fall back to
    // a generic three-array (a, b, c) template with a note.
    bool genericFallback = arrays.empty();

    // ── 2. Separate inputs from outputs ──────────────────────────────────
    std::vector<ArrayInfo*> inputs, outputs;
    for(auto& [n, a] : arrays){
        if(a.written) outputs.push_back(&a);
        else          inputs.push_back(&a);
    }
    // An array that is both read and written counts as an in-out → output list.
    // Remove from inputs if also in outputs.
    std::set<std::string> outNames;
    for(auto* a : outputs) outNames.insert(a->name);
    inputs.erase(std::remove_if(inputs.begin(), inputs.end(),
                                [&](ArrayInfo* a){ return outNames.count(a->name); }),
                 inputs.end());

    // ── 3. Write the .cu file ─────────────────────────────────────────────
    std::ofstream cu(cuFile);
    if(!cu){ std::cerr<<"Error: cannot open "<<cuFile<<"\n"; return; }

    cu << "// Generated CUDA kernel by BulkCompiler\n";
    cu << "// Loop bound: " << pl.bound << "\n";
    cu << "#include <stdio.h>\n";
    cu << "#include <stdlib.h>\n";
    cu << "#include <cuda_runtime.h>\n\n";

    // ── 3a. Build kernel parameter list ──────────────────────────────────
    // Pattern:  (T* in0, T* in1, ..., T* out0, ..., int N)
    auto arrayC = [](IRType t) -> std::string {
        switch(t){
            case IRType::INT64:  return "long long";
            case IRType::FLOAT:  return "float";
            case IRType::CHAR:   return "char";
            default:             return "int";
        }
    };

    std::string kernelParams;
    std::vector<std::string> allArrayNames;   // ordered: inputs then outputs

    if(genericFallback){
        // Generic three-array fallback
        kernelParams = "int *a, int *b, int *c, int N";
        allArrayNames = {"a","b","c"};
    } else {
        bool first = true;
        for(auto* a : inputs){
            if(!first) kernelParams += ", ";
            kernelParams += arrayC(a->type) + "* " + a->name;
            allArrayNames.push_back(a->name);
            first = false;
        }
        for(auto* a : outputs){
            if(!first) kernelParams += ", ";
            kernelParams += arrayC(a->type) + "* " + a->name;
            allArrayNames.push_back(a->name);
            first = false;
        }
        kernelParams += ", int N";
    }

    // ── 3b. Kernel function ───────────────────────────────────────────────
    cu << "__global__ void bulkKernel(" << kernelParams << ") {\n";
    cu << "    int " << pl.idxVar << " = blockIdx.x * blockDim.x + threadIdx.x;\n";
    cu << "    if (" << pl.idxVar << " < N) {\n";

    if(genericFallback){
        // Generic element-wise add as placeholder; mark clearly
        cu << "        // TODO: replace with actual loop body\n";
        cu << "        c[" << pl.idxVar << "] = a[" << pl.idxVar
           << "] + b[" << pl.idxVar << "];\n";
    } else {
        // Emit body instructions using the same emitC helper.
        // We suppress loop-control labels/gotos (they're now handled by CUDA).
        std::set<std::string> skipLbls;
        skipLbls.insert(pl.Lstart); skipLbls.insert(pl.Lend);
        if(!pl.Lstep.empty()) skipLbls.insert(pl.Lstep);

        for(size_t i=pl.bodyStart; i<pl.bodyEnd && i<ir.size(); ++i){
            auto& ins = ir[i];
            if(ins.op=="label" && skipLbls.count(ins.arg1)) continue;
            if(ins.op=="goto"  && skipLbls.count(ins.arg1)) continue;
            if(ins.op=="ifzero_goto" && skipLbls.count(ins.arg2)) continue;
            if(ins.op=="func_begin"||ins.op=="func_end"||ins.op=="param") continue;
            // push_arg / call inside a kernel body — emit inline
            std::string line = emitC(ins, "        ");
            if(!line.empty()) cu << line << "\n";
        }
    }

    cu << "    }\n}\n\n";

    // ── 3c. Host main() ───────────────────────────────────────────────────
    cu << "int main() {\n";
    cu << "    const int N = " << pl.bound << ";\n\n";

    // Host allocations
    if(genericFallback){
        cu << "    int *a = (int*)malloc(N * sizeof(int));\n";
        cu << "    int *b = (int*)malloc(N * sizeof(int));\n";
        cu << "    int *c = (int*)malloc(N * sizeof(int));\n\n";
        cu << "    for (int i = 0; i < N; i++) { a[i] = i; b[i] = i * 2; }\n\n";
    } else {
        for(auto* a : inputs){
            cu << "    " << arrayC(a->type) << " *" << a->name
               << " = (" << arrayC(a->type) << "*)malloc(N * sizeof("
               << arrayC(a->type) << "));\n";
        }
        for(auto* a : outputs){
            cu << "    " << arrayC(a->type) << " *" << a->name
               << " = (" << arrayC(a->type) << "*)malloc(N * sizeof("
               << arrayC(a->type) << "));\n";
        }
        cu << "\n";
        // Simple initialisation for inputs
        if(!inputs.empty()){
            cu << "    for (int i = 0; i < N; i++) {\n";
            int seed = 1;
            for(auto* a : inputs)
                cu << "        " << a->name << "[i] = i * " << seed++ << ";\n";
            cu << "    }\n\n";
        }
    }

    // Device allocations
    for(auto& n : allArrayNames)
        cu << "    " << arrayC(arrays.count(n)?arrays[n].type:IRType::INT32)
           << " *d_" << n << ";\n";
    cu << "\n";
    for(auto& n : allArrayNames)
        cu << "    cudaMalloc(&d_" << n << ", N * sizeof("
           << arrayC(arrays.count(n)?arrays[n].type:IRType::INT32) << "));\n";
    cu << "\n";

    // Copy inputs to device
    for(auto* a : inputs)
        cu << "    cudaMemcpy(d_" << a->name << ", " << a->name
           << ", N * sizeof(" << arrayC(a->type) << "), cudaMemcpyHostToDevice);\n";
    if(genericFallback){
        cu << "    cudaMemcpy(d_a, a, N * sizeof(int), cudaMemcpyHostToDevice);\n";
        cu << "    cudaMemcpy(d_b, b, N * sizeof(int), cudaMemcpyHostToDevice);\n";
    }
    cu << "\n";

    // Kernel launch
    cu << "    int threads = 256;\n";
    cu << "    int blocks  = (N + threads - 1) / threads;\n";
    cu << "    bulkKernel<<<blocks, threads>>>(";
    bool firstArg = true;
    for(auto& n : allArrayNames){
        if(!firstArg) cu << ", ";
        cu << "d_" << n;
        firstArg = false;
    }
    cu << ", N);\n";
    cu << "    cudaDeviceSynchronize();\n\n";

    // Copy outputs back
    for(auto* a : outputs)
        cu << "    cudaMemcpy(" << a->name << ", d_" << a->name
           << ", N * sizeof(" << arrayC(a->type) << "), cudaMemcpyDeviceToHost);\n";
    if(genericFallback)
        cu << "    cudaMemcpy(c, d_c, N * sizeof(int), cudaMemcpyDeviceToHost);\n";
    cu << "\n";

    // Sample output
    if(genericFallback)
        cu << "    printf(\"c[10] = %d\\n\", c[10]);\n\n";
    else if(!outputs.empty())
        cu << "    printf(\"" << outputs[0]->name << "[10] = "
           << (outputs[0]->type==IRType::FLOAT?"%f":"%d") << "\\n\", "
           << outputs[0]->name << "[10]);\n\n";

    // Cleanup
    for(auto& n : allArrayNames)
        cu << "    cudaFree(d_" << n << ");\n";
    if(genericFallback){
        cu << "    free(a); free(b); free(c);\n";
    } else {
        for(auto* a : inputs)  cu << "    free(" << a->name << ");\n";
        for(auto* a : outputs) cu << "    free(" << a->name << ");\n";
    }

    cu << "    return 0;\n}\n";
    cu.close();

    std::cout << "[Codegen] CUDA kernel written to: " << cuFile << "\n";
    std::string gpuExe = exeName + "_gpu";
    // Attempt compilation with nvcc
    std::string cmd = "nvcc " + cuFile + " -o " + gpuExe + " 2>&1";
    std::cout << "[Codegen] Compiling CUDA: " << cmd << "\n";
    int ret = system(cmd.c_str());
    if(ret == 0)
        std::cout << "[Codegen] Success! Run with: ./" << gpuExe << "\n";
    else
        std::cerr << "[Codegen] nvcc failed — ensure CUDA toolkit is installed\n";
}

// ── Emit one IR instruction as C ─────────────────────────────────────────────
static std::string emitC(const IRInstruction& ins, const std::string& ind) {
    if(ins.op=="label")       return ins.arg1+":;";
    if(ins.op=="goto")        return ind+"goto "+ins.arg1+";";
    if(ins.op=="ifzero_goto") return ind+"if(!("+ins.arg1+")) goto "+ins.arg2+";";
    if(ins.op=="comment")     return ind+"// "+ins.arg1;
    if(ins.op=="param"||ins.op=="push_arg"||ins.op=="func_begin"||ins.op=="func_end") return "";
    if(ins.op=="return"){
        if(ins.arg1.empty()) return ind+"return;";
        return ind+"return "+ins.arg1+";";
    }
    if(ins.op=="scan"){
        IRType t=symtab.exists(ins.arg1)?symtab.get(ins.arg1).irType:IRType::INT32;
        return ind+"scanf(\""+scanFmt(t)+"\", &"+ins.arg1+");";
    }
    if(ins.op=="print"||ins.op=="println"){
        std::string nl=(ins.op=="println")?"\\n":"";
        return ind+"printf(\""+printFmt(ins.type)+nl+"\", "+ins.arg1+");";
    }
    if(ins.op=="str_const"){
        return ind+ins.result+" = (int)(intptr_t)\""+ins.arg1+"\";";
    }
    if(ins.op=="alloc"){
        return ind+ins.result+" = (int)(intptr_t)malloc(sizeof("+ins.arg1+")*"+ins.arg2+");";
    }
    if(ins.op=="free"){
        return ind+"free((void*)(intptr_t)"+ins.arg1+");";
    }
    if(ins.op=="addr_of")  return ind+ins.result+" = (int)(intptr_t)&"+ins.arg1+";";
    if(ins.op=="deref")    return ind+ins.result+" = *(int*)(intptr_t)"+ins.arg1+";";
    if(ins.op=="field_read") return ind+ins.result+" = "+ins.arg1+"."+ins.arg2+";";
    if(ins.op=="cast"){
        return ind+ins.result+" = ("+cType(ins.type)+")"+ins.arg1+";";
    }
    if(ins.op=="="){
        if(ins.arg2.empty()) return ind+ins.result+" = "+ins.arg1+";";
        return ind+ins.result+" = "+ins.arg1+";";
    }
    if(ins.op=="neg") return ind+ins.result+" = -"+ins.arg2+";";
    if(ins.op=="~")   return ind+ins.result+" = ~"+ins.arg2+";";
    return ind+ins.result+" = "+ins.arg1+" "+ins.op+" "+ins.arg2+";";
}

// ── Emit a range of IR as C ───────────────────────────────────────────────────
static void emitRange(std::ostream& out, size_t start, size_t end,
                      const std::string& ind,
                      bool useCUDA=false,
                      const std::string& exeName="",
                      const std::string& cuFile="",
                      const std::set<std::string>& skipLabels={})
{
    std::vector<std::string> pendingArgs;
    for(size_t i=start; i<end&&i<ir.size(); ++i){
        auto& ins=ir[i];

        if(ins.op=="comment"&&isParallelComment(ins.arg1)){
            ParallelLoop pl=detectParallelLoop(i);
            if(pl.found){
                std::cerr << "[DEBUG] Parallel loop detected: idxVar=" << pl.idxVar
                          << ", bound=" << pl.bound
                          << ", useCUDA=" << useCUDA
                          << ", isLarge=" << isLargeLoop(pl.bound) << "\n";

                if(useCUDA && isLargeLoop(pl.bound)){
                    // Emit CUDA kernel and note in the C file
                    emitCUDAKernel(pl, cuFile, exeName);
                    out << ind << "// CUDA kernel emitted for loop bound=" << pl.bound << "\n";
                    out << ind << "// See: " << cuFile << "\n";
                } else {
                    std::set<std::string> innerSkip;
                    innerSkip.insert(pl.Lstart);
                    innerSkip.insert(pl.Lend);
                    if(!pl.Lstep.empty()) innerSkip.insert(pl.Lstep);

                    out<<"\n"<<ind<<"// auto-parallelised by BulkCompiler\n";
                    out<<ind<<"#pragma omp parallel for schedule(static)\n";
                    out<<ind<<"for(int "<<pl.idxVar<<"=0; "
                       <<pl.idxVar<<"<"<<pl.bound<<"; ++"<<pl.idxVar<<") {\n";
                    emitRange(out, pl.bodyStart, pl.bodyEnd,
                              ind+"    ", useCUDA, exeName, cuFile, innerSkip);
                    out<<ind<<"}\n";
                }
                i=pl.lendIdx; continue;
            }
        }

        if(ins.op=="func_begin"||ins.op=="func_end"||ins.op=="param") continue;
        if(ins.op=="push_arg"){ pendingArgs.push_back(ins.arg1); continue; }

        if(ins.op=="call"){
            std::string al;
            for(size_t a=0;a<pendingArgs.size();++a){if(a) al+=", "; al+=pendingArgs[a];}
            pendingArgs.clear();
            if(ins.result.empty()) out<<ind<<ins.arg1<<"("<<al<<");\n";
            else out<<ind<<ins.result<<" = "<<ins.arg1<<"("<<al<<");\n";
            continue;
        }

        if(!skipLabels.empty()){
            if(ins.op=="label" && skipLabels.count(ins.arg1)) continue;
            if(ins.op=="goto"  && skipLabels.count(ins.arg1)) continue;
            if(ins.op=="ifzero_goto" && skipLabels.count(ins.arg2)) continue;
        }

        std::string line=emitC(ins,ind);
        if(!line.empty()) out<<line<<"\n";
    }
}

// ── Main entry point ──────────────────────────────────────────────────────────
void generateCode(const std::string& cFile, const std::string& exeName, bool useCUDA) {
    std::string cuFile = exeName + ".cu";
 
    // Pass 1: find function boundaries
    struct FuncRange { size_t begin, end; std::string name; IRType ret; };
    std::vector<FuncRange> funcs;
    std::set<size_t> inFuncIdx;
 
    for (size_t i = 0; i < ir.size(); ++i) {
        if (ir[i].op == "func_begin") {
            FuncRange fr;
            fr.name  = ir[i].arg1;
            fr.ret   = ir[i].type;
            fr.begin = i;
            for (size_t j = i+1; j < ir.size(); ++j) {
                if (ir[j].op == "func_end" && ir[j].arg1 == fr.name) {
                    fr.end = j;
                    funcs.push_back(fr);
                    i = j;
                    break;
                }
            }
        }
    }
    for (auto& fr : funcs)
        for (size_t k = fr.begin; k <= fr.end; ++k)
            inFuncIdx.insert(k);
 
    // Build global index list
    std::vector<size_t> globalIdxs;
    for (size_t i = 0; i < ir.size(); ++i)
        if (!inFuncIdx.count(i)) globalIdxs.push_back(i);
 
    std::ofstream out(cFile);
    if (!out) { std::cerr << "Error: cannot open " << cFile << "\n"; return; }
 
    out << "// Generated by BulkCompiler\n";
    out << "// Compile: gcc -O2 -fopenmp " << cFile << " -o " << exeName << "\n\n";
    out << "#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n#include <omp.h>\n\n";
 
    // Forward declarations
    for (auto& fr : funcs) {
        std::string params;
        for (size_t j = fr.begin+1; j < ir.size() && ir[j].op == "param"; ++j) {
            if (!params.empty()) params += ", ";
            params += cType(ir[j].type) + " " + ir[j].arg1;
        }
        out << cType(fr.ret) << " " << fr.name << "(" << params << ");\n";
    }
    if (!funcs.empty()) out << "\n";
 
    // Global variable declarations
    for (auto& [name, sym] : symtab.globalSymbols()) {
        if (sym.isArray) {
            out << cType(sym.irType) << " " << name;
            for (auto dim : sym.dimensions) out << "[" << dim << "]";
            out << " = {0};\n";
        } else {
            out << cType(sym.irType) << " " << name << " = 0;\n";
        }
    }
    out << "\n";
 
    // Function definitions
    for (auto& fr : funcs) {
        std::string params;
        size_t bodyStart = fr.begin + 1;
        while (bodyStart < ir.size() && ir[bodyStart].op == "param") {
            if (!params.empty()) params += ", ";
            params += cType(ir[bodyStart].type) + " " + ir[bodyStart].arg1;
            ++bodyStart;
        }
        out << cType(fr.ret) << " " << fr.name << "(" << params << ") {\n";
        auto temps = collectTemps(bodyStart, fr.end);
        for (auto& [n, t] : temps)
            out << "    " << cType(t) << " " << n << " = 0;\n";
        if (!temps.empty()) out << "\n";
        emitRange(out, bodyStart, fr.end, "    ", useCUDA, exeName, cuFile);
        out << "}\n\n";
    }
 
    // main()
    out << "int main(int argc, char* argv[]) {\n";
 
    auto allTemps = collectTemps(0, ir.size());
    for (auto& fr : funcs) {
        auto ft = collectTemps(fr.begin, fr.end+1);
        for (auto& [k, v] : ft) allTemps.erase(k);
    }
    for (auto& [n, t] : allTemps)
        out << "    " << cType(t) << " " << n << " = 0;\n";
    if (!allTemps.empty()) out << "\n";
 
    // Emit global-scope IR
    std::vector<std::string> pendingArgs;
    size_t gi = 0;
    while (gi < globalIdxs.size()) {
        size_t i = globalIdxs[gi];
        auto& ins = ir[i];
 
        if (ins.op == "comment" && isParallelComment(ins.arg1)) {
            ParallelLoop pl = detectParallelLoop(i);
            if (pl.found) {
                std::cerr << "[DEBUG] Parallel loop detected (global): idxVar=" << pl.idxVar
                          << ", bound=" << pl.bound
                          << ", useCUDA=" << useCUDA
                          << ", isLarge=" << isLargeLoop(pl.bound) << "\n";
 
                // ── CUDA: emit .cu kernel in ADDITION to the .c loop ──────
                if (useCUDA && isLargeLoop(pl.bound)) {
                    emitCUDAKernel(pl, cuFile, exeName);
                    out << "    // CUDA kernel emitted for loop bound=" << pl.bound << "\n";
                    out << "    // See: " << cuFile << "\n";
                }
 
                // ── Always emit the OpenMP C for-loop in the .c file ──────
                std::set<std::string> skipLabels;
                skipLabels.insert(pl.Lstart);
                skipLabels.insert(pl.Lend);
                if (!pl.Lstep.empty()) skipLabels.insert(pl.Lstep);
 
                out << "\n    // auto-parallelised by BulkCompiler\n";
                out << "    #pragma omp parallel for schedule(static)\n";
                out << "    for (int " << pl.idxVar << " = 0; "
                    << pl.idxVar << " < " << pl.bound << "; "
                    << "++" << pl.idxVar << ") {\n";
                emitRange(out, pl.bodyStart, pl.bodyEnd,
                          "        ", useCUDA, exeName, cuFile, skipLabels);
                out << "    }\n";
 
                while (gi < globalIdxs.size() && globalIdxs[gi] <= pl.lendIdx) ++gi;
                continue;
            }
        }
 
        if (ins.op == "func_begin" || ins.op == "func_end" || ins.op == "param") {
            ++gi; continue;
        }
        if (ins.op == "push_arg") { pendingArgs.push_back(ins.arg1); ++gi; continue; }
        if (ins.op == "call") {
            std::string al;
            for (size_t a = 0; a < pendingArgs.size(); ++a) {
                if (a) al += ", ";
                al += pendingArgs[a];
            }
            pendingArgs.clear();
            if (ins.result.empty())
                out << "    " << ins.arg1 << "(" << al << ");\n";
            else
                out << "    " << ins.result << " = " << ins.arg1 << "(" << al << ");\n";
            ++gi; continue;
        }
 
        std::string line = emitC(ins, "    ");
        if (!line.empty()) out << line << "\n";
        ++gi;
    }

    std::string cpuExe = exeName + "_cpu";
    std::string gpuExe = exeName + "_gpu";
    out << "    return 0;\n}\n";
    out.close();
 
    std::cout << "[Codegen] C file written to: " << cFile << "\n";
 
    if (useCUDA) {
        std::cout << "[Codegen] CUDA mode active. Large loops (>10000) also emit .cu kernels.\n";
        //std::cout << "[Codegen] Link CUDA binary with: nvcc " << cuFile << " -o " << gpuExe << "\n";
    }
 
    std::string cmd = "gcc -O2 -fopenmp " + cFile + " -o " + cpuExe + " 2>&1";
    std::cout << "[Codegen] Compiling C: " << cmd << "\n";
    int ret = system(cmd.c_str());
    if (ret == 0)
        std::cout << "[Codegen] Success! Run with: ./" << cpuExe << "\n";
    else
        std::cerr << "[Codegen] gcc failed — see errors above\n";
}

// ── Assembly generation ───────────────────────────────────────────────────────
void generateAssembly(const std::string& cFile, const std::string& outFile,
                      const std::string& arch)
{
    std::string flag;
    std::string compiler;
    if(arch=="x86")       flag="-m32";
    else if(arch=="x86_64") flag="";
    else if(arch=="arm")    {compiler = "arm-linux-gnueabi-gcc"; flag = "";}
    else if(arch=="riscv")  {compiler = "riscv64-linux-gnu-gcc"; flag = "-march=rv64gc -mabi=lp64d";}

    std::string cmd="gcc -S -O2 "+flag+" "+cFile+" -o "+outFile+" 2>&1";
    std::cout<<"[ASM] Generating "<<arch<<" assembly: "<<cmd<<"\n";
    int r=system(cmd.c_str());
    if(r==0) std::cout<<"[ASM] Assembly written to: "<<outFile<<"\n";
    else std::cerr<<"[ASM] Failed\n";
}
// Add this helper function at the top of codegen.cpp
std::string typeToString(IRType t) {
    switch(t) {
        case IRType::INT32:  return "i32";
        case IRType::INT64:  return "i64";
        case IRType::INT128: return "i128";
        case IRType::FLOAT:  return "f32";
        case IRType::CHAR:   return "char";
        case IRType::BOOL:   return "bool";
        case IRType::VOID:   return "void";
        default:             return "";
    }
}

void generateCFG(const std::string& dotFile)
{
    std::ofstream out(dotFile);
    if(!out){std::cerr<<"Error: cannot open "<<dotFile<<"\n";return;}

    out<<"digraph CFG {\n";
    out<<"  node [shape=box fontname=\"Courier\" fontsize=10];\n";
    out<<"  rankdir=TB;\n";

    struct Block {
        std::string id;
        std::vector<std::string> instrs;
        std::vector<std::string> succs;
    };
    std::vector<Block> blocks;
    std::unordered_map<std::string,int> labelToBlock;

    // ===== STEP 1: Reconstruct full instruction strings =====
    std::vector<std::string> fullInstructions;

    for(size_t i = 0; i < ir.size(); ++i) {
        std::string line;
        const auto& ins = ir[i];

        if(ins.op == "label") {
            line = ins.arg1 + ":";
        }
        else if(ins.op == "ifzero_goto") {
            line = "if " + ins.arg1 + " == 0 goto " + ins.arg2;
        }
        else if(ins.op == "goto") {
            line = "goto " + ins.arg1;
        }
        else if(ins.op == "return") {
            line = "return";
            if(!ins.arg1.empty()) line += " " + ins.arg1;
        }
        else if(ins.op == "comment") {
            // BUG FIX 3: do NOT strip brackets from comments —
            // keep the full comment text as-is.
            line = "// " + ins.arg1;
        }
        else if(ins.op == "print" || ins.op == "println") {
            line = ins.op + " " + ins.arg1;
        }
        else if(ins.op == "scan") {
            line = "scan " + ins.arg1;
        }
        // BUG FIX 1: handle array store  a[i] = value
        // IR stores array name in result, value in arg1, op == "="
        // but symtab tells us it's an array. Reconstruct as a[idx] = val.
        else if(ins.op == "=" && !ins.result.empty()) {
            // Check whether result is an array element assignment.
            // The IR for  a[i] = t5  typically comes right after  t_addr = a + i
            // and is emitted as:  result="a[i]"  arg1="t5"  in many compilers,
            // OR as result="a" with a separate index temp.
            // Here we just reconstruct what we have faithfully:
            if(!ins.arg2.empty())
                line = ins.result + " = " + ins.arg1;   // arg2 unused in plain "="
            else
                line = ins.result + " = " + ins.arg1;
        }
        else if(!ins.result.empty()) {
            // Binary / unary op with result
            if(ins.arg2.empty())
                line = ins.result + " = " + ins.op + " " + ins.arg1;
            else
                line = ins.result + " = " + ins.arg1 + " " + ins.op + " " + ins.arg2;
        }
        else {
            // Ops with no result (func_begin, func_end, param, push_arg, call …)
            line = ins.op;
            if(!ins.arg1.empty()) line += " " + ins.arg1;
            if(!ins.arg2.empty()) line += ", " + ins.arg2;
        }

        // Append type annotation ONLY to non-control-flow, non-comment lines.
        // BUG FIX 3 (continued): skip annotation for comments entirely so we
        // never accidentally strip their bracket content later.
        if(ins.op != "label"  && ins.op != "goto"  &&
           ins.op != "ifzero_goto" && ins.op != "comment" &&
           ins.op != "return" && ins.type != IRType::VOID)
        {
            std::string ts = typeToString(ins.type);
            if(!ts.empty()) line += " [" + ts + "]";
        }

        fullInstructions.push_back(line);
    }

    std::cout << "[CFG] Reconstructed IR (" << fullInstructions.size() << " instructions):\n";
    for(size_t i = 0; i < fullInstructions.size() && i < 30; ++i)
        std::cout << "  " << i << ": " << fullInstructions[i] << "\n";

    // ===== STEP 2: Find basic block leaders =====
    std::set<size_t> leaders;
    leaders.insert(0);

    for(size_t i = 0; i < fullInstructions.size(); ++i) {
        const std::string& ln = fullInstructions[i];

        bool isUncondGoto = (ln.find("goto") != std::string::npos &&
                             ln.find("if ")  == std::string::npos);
        bool isCondGoto   = (ln.rfind("if ", 0) == 0 &&
                             ln.find("goto") != std::string::npos);
        bool isReturn     = (ln == "return" || ln.rfind("return ", 0) == 0);
        bool isLabel      = (!ln.empty() && ln.back() == ':');

        if((isUncondGoto || isCondGoto || isReturn) && i+1 < fullInstructions.size())
            leaders.insert(i + 1);
        if(isLabel)
            leaders.insert(i);
    }

    // ===== STEP 3: Build basic blocks =====
    // BUG FIX 2: build the labelToBlock map in a FIRST pass over blocks
    // before we need to resolve successors, so every label is known.

    //int bn = 0;
    std::vector<std::pair<size_t,size_t>> blockRanges; // [start, end)
    for(auto it = leaders.begin(); it != leaders.end(); ++it) {
        auto nx = std::next(it);
        size_t end = (nx != leaders.end()) ? *nx : fullInstructions.size();
        blockRanges.push_back({*it, end});
    }

    // First pass: assign block IDs and populate labelToBlock
    for(size_t b = 0; b < blockRanges.size(); ++b) {
        Block blk;
        blk.id = "B" + std::to_string(b);
        auto [bstart, bend] = blockRanges[b];

        for(size_t j = bstart; j < bend; ++j) {
            const std::string& ln = fullInstructions[j];
            if(ln.empty()) continue;

            // Map every label in this block to this block index.
            // BUG FIX 2: use `b` (the real block index) not `bn-1`.
            if(!ln.empty() && ln.back() == ':') {
                std::string label = ln.substr(0, ln.size() - 1);
                labelToBlock[label] = static_cast<int>(b);
            }

            // Strip type annotation brackets for cleaner display,
            // but ONLY from lines that cannot be comments.
            // BUG FIX 3: comments start with "//" — leave them alone.
            std::string display = ln;
            if(ln.rfind("//", 0) != 0) {
                size_t bp = display.rfind(" [");
                if(bp != std::string::npos) {
                    std::string suffix = display.substr(bp + 2);
                    // Only strip if suffix looks like a type token (no spaces, ends with ']')
                    if(suffix.back() == ']' && suffix.find(' ') == std::string::npos)
                        display = display.substr(0, bp);
                }
            }
            blk.instrs.push_back(display);
        }
        blocks.push_back(blk);
    }

    // ===== STEP 4: Determine successors =====
    for(size_t b = 0; b < blocks.size(); ++b) {
        auto& blk = blocks[b];
        if(blk.instrs.empty()) {
            if(b+1 < blocks.size()) blk.succs.push_back("B"+std::to_string(b+1));
            continue;
        }

        // Find the last non-comment, non-label instruction
        std::string lastLine;
        for(int k = (int)blk.instrs.size()-1; k >= 0; --k) {
            const std::string& s = blk.instrs[k];
            if(!s.empty() && s.rfind("//",0) != 0 && s.back() != ':') {
                lastLine = s;
                break;
            }
        }

        bool isUncond = (lastLine.find("goto") != std::string::npos &&
                         lastLine.find("if ")  == std::string::npos);
        bool isCond   = (lastLine.rfind("if ", 0) == 0 &&
                         lastLine.find("goto") != std::string::npos);
        bool isRet    = (lastLine == "return" || lastLine.rfind("return ", 0) == 0);

        auto extractTarget = [](const std::string& ln) -> std::string {
            size_t gp = ln.find("goto");
            if(gp == std::string::npos) return "";
            std::string t = ln.substr(gp + 4);
            t.erase(0, t.find_first_not_of(" \t"));
            auto end = t.find_first_of(" \t;");
            if(end != std::string::npos) t = t.substr(0, end);
            return t;
        };

        if(isRet) {
            // no successors
        } else if(isUncond) {
            std::string tgt = extractTarget(lastLine);
            auto it = labelToBlock.find(tgt);
            if(it != labelToBlock.end())
                blk.succs.push_back("B" + std::to_string(it->second));
        } else if(isCond) {
            // Fall-through (condition false)
            if(b+1 < blocks.size())
                blk.succs.push_back("B" + std::to_string(b+1));
            // Jump target (condition true)
            std::string tgt = extractTarget(lastLine);
            auto it = labelToBlock.find(tgt);
            if(it != labelToBlock.end())
                blk.succs.push_back("B" + std::to_string(it->second));
        } else {
            // Normal fall-through
            if(b+1 < blocks.size())
                blk.succs.push_back("B" + std::to_string(b+1));
        }
    }

    // ===== STEP 5: Write DOT file =====
    for(auto& b : blocks) {
        out << "  " << b.id << " [label=\"" << b.id << "\\n";
        for(auto& ins : b.instrs) {
            std::string esc = ins;
            // Escape backslashes first (before we add any), then quotes
            for(size_t p=0; (p=esc.find('\\',p))!=std::string::npos; p+=2)
                esc.replace(p, 1, "\\\\");
            for(size_t p=0; (p=esc.find('"', p))!=std::string::npos; p+=2)
                esc.replace(p, 1, "\\\"");
            out << esc << "\\l";
        }
        out << "\"];\n";
    }

    for(auto& b : blocks)
        for(auto& s : b.succs)
            out << "  " << b.id << " -> " << s << ";\n";

    out << "}\n";
    out.close();
    std::cout << "[CFG] Dot file written to: " << dotFile << "\n";

    std::string cmd = "dot -Tpng " + dotFile + " -o " + dotFile + ".png 2>&1";
    int r = system(cmd.c_str());
    if(r == 0) std::cout << "[CFG] PNG rendered to: " << dotFile << ".png\n";
    else std::cerr << "[CFG] To render PNG, install Graphviz and run: dot -Tpng "
                   << dotFile << " -o " << dotFile << ".png\n";
}
// ── Optimization report ───────────────────────────────────────────────────────
void generateOptReport(const std::string& outFile, size_t before, size_t after, int level)
{
    std::ofstream out(outFile);
    out<<"=== Optimization Report ===\n";
    out<<"Level: -O"<<level<<"\n";
    out<<"IR instructions before: "<<before<<"\n";
    out<<"IR instructions after:  "<<after<<"\n";
    if(before>after) out<<"Removed: "<<(before-after)<<" instructions\n";
    else out<<"No instructions removed\n";
    out<<"\nPasses applied:\n";
    if(level>=1){
        out<<"  [x] Constant folding\n";
        out<<"  [x] Constant propagation\n";
        out<<"  [x] Copy propagation\n";
        out<<"  [x] Common subexpression elimination\n";
        out<<"  [x] Loop-invariant code motion\n";
        out<<"  [x] Dead code elimination\n";
    }
    if(level>=2){
        out<<"  [x] All O1 passes repeated until stable\n";
        out<<"  [x] Peephole optimization\n";
    }
    out.close();
    std::cout<<"[Opt] Report written to: "<<outFile<<"\n";
}