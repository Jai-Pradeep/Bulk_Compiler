

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
    std::string idxVar,bound,Lstart,Lend;
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
    pl.bodyEnd=i; std::string tStep=ir[i].result; ++i;
    if(i>=ir.size()||ir[i].op!="="||ir[i].arg1!=tStep) return pl;
    ++i;
    if(i>=ir.size()||ir[i].op!="goto"||ir[i].arg1!=pl.Lstart) return pl;
    pl.gotoIdx=i; ++i;
    if(i>=ir.size()||ir[i].op!="label"||ir[i].arg1!=pl.Lend) return pl;
    pl.lendIdx=i; pl.found=true; return pl;
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
    if(ins.op=="scan"){
        return ind+"scanf(\""+scanFmt(ins.type)+"\", &"+ins.arg1+");";
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
        std::string fromStr=ins.arg2; // "i32->i64" etc.
        return ind+ins.result+" = ("+cType(ins.type)+")"+ins.arg1+";";
    }
    // if(ins.op=="break")    return ind+"break;";
    // if(ins.op=="continue") return ind+"continue;";
    if(ins.op=="="){
        if(ins.arg2.empty()) return ind+ins.result+" = "+ins.arg1+";";
        return ind+ins.result+" = "+ins.arg1+";";
    }
    if(ins.op=="neg") return ind+ins.result+" = -"+ins.arg2+";";
    if(ins.op=="~") return ind+ins.result+" = ~"+ins.arg2+";";
    // binary / comparison
    return ind+ins.result+" = "+ins.arg1+" "+ins.op+" "+ins.arg2+";";
}

// ── Emit a range of IR as C, handling parallel loops and calls ───────────────
static void emitRange(std::ostream& out, size_t start, size_t end,
                      const std::string& ind, bool useCUDA=false,
                      const std::string& exeName="", const std::string& cuFile="")
{
    std::vector<std::string> pendingArgs;
    for(size_t i=start; i<end&&i<ir.size(); ++i){
        auto& ins=ir[i];
        if(ins.op=="comment"&&isParallelComment(ins.arg1)){
            ParallelLoop pl=detectParallelLoop(i);
            if(pl.found){
                if(useCUDA&&isLargeLoop(pl.bound)){
                    out<<ind<<"// CUDA kernel would run here for loop bound="<<pl.bound<<"\n";
                } else {
                    out<<"\n"<<ind<<"#pragma omp parallel for schedule(static)\n";
                    out<<ind<<"for(int "<<pl.idxVar<<"=0; "
                        <<pl.idxVar<<"<"<<pl.bound<<"; ++"<<pl.idxVar<<") {\n";
                    emitRange(out, pl.bodyStart, pl.bodyEnd, ind+"    ", useCUDA, exeName, cuFile);
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
        std::string line=emitC(ins,ind);
        if(!line.empty()) out<<line<<"\n";
    }
}

// ── Main code generation ──────────────────────────────────────────────────────
void generateCode(const std::string& cFile, const std::string& exeName, bool useCUDA)
{
    std::string cuFile=exeName+".cu";

    // Identify function ranges
    struct FuncRange { std::string name; IRType ret; size_t begin, end; };
    std::vector<FuncRange> funcs;
    std::set<size_t> inFuncIdx;
    for(size_t i=0;i<ir.size();++i){
        if(ir[i].op=="func_begin"){
            FuncRange fr; fr.name=ir[i].arg1; fr.ret=ir[i].type; fr.begin=i;
            for(size_t j=i+1;j<ir.size();++j){
                if(ir[j].op=="func_end"&&ir[j].arg1==fr.name){fr.end=j;funcs.push_back(fr);i=j;break;}
            }
        }
    }
    for(auto& fr:funcs) for(size_t k=fr.begin;k<=fr.end;++k) inFuncIdx.insert(k);

    // Build global index list
    std::vector<size_t> globalIdxs;
    for(size_t i=0;i<ir.size();++i) if(!inFuncIdx.count(i)) globalIdxs.push_back(i);

    std::ofstream out(cFile);
    if(!out){std::cerr<<"Error: cannot open "<<cFile<<"\n"; return;}

    out<<"// Generated by BulkCompiler\n";
    out<<"// Compile: gcc -O2 -fopenmp "<<cFile<<" -o "<<exeName<<"\n\n";
    out<<"#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n#include <omp.h>\n\n";

    // Forward declarations
    for(auto& fr:funcs){
        std::string params;
        for(size_t j=fr.begin+1;j<ir.size()&&ir[j].op=="param";++j){
            if(!params.empty()) params+=", ";
            params+=cType(ir[j].type)+" "+ir[j].arg1;
        }
        out<<cType(fr.ret)<<" "<<fr.name<<"("<<params<<");\n";
    }
    if(!funcs.empty()) out<<"\n";

    // Global variable declarations
    for(auto& [name,sym]:symtab.globalSymbols()){
        if(sym.isArray){
            // Emit: int mat[3][4] = {0};
            out<<cType(sym.irType)<<" "<<name;
            for(auto dim : sym.dimensions)
                out<<"["<<dim<<"]";
            out<<" = {0};\n";
        } else {
            out<<cType(sym.irType)<<" "<<name<<" = 0;\n";
        }
    }
    out<<"\n";

    // Function definitions
    for(auto& fr:funcs){
        std::string params;
        size_t bodyStart=fr.begin+1;
        while(bodyStart<ir.size()&&ir[bodyStart].op=="param"){
            if(!params.empty()) params+=", ";
            params+=cType(ir[bodyStart].type)+" "+ir[bodyStart].arg1;
            ++bodyStart;
        }
        out<<cType(fr.ret)<<" "<<fr.name<<"("<<params<<") {\n";
        auto temps=collectTemps(bodyStart,fr.end);
        for(auto& [n,t]:temps) out<<"    "<<cType(t)<<" "<<n<<" = 0;\n";
        if(!temps.empty()) out<<"\n";
        emitRange(out,bodyStart,fr.end,"    ",useCUDA,exeName,cuFile);
        out<<"}\n\n";
    }

    // main()
    out<<"int main(int argc, char* argv[]) {\n";

    // Declare global-scope temps (excluding function temps)
    auto allTemps=collectTemps(0,ir.size());
    for(auto& fr:funcs){
        auto ft=collectTemps(fr.begin,fr.end+1);
        for(auto& [k,v]:ft) allTemps.erase(k);
    }
    for(auto& [n,t]:allTemps) out<<"    "<<cType(t)<<" "<<n<<" = 0;\n";
    if(!allTemps.empty()) out<<"\n";

    // Emit global-scope IR
    std::vector<std::string> pendingArgs;
    size_t gi=0;
    while(gi<globalIdxs.size()){
        size_t i=globalIdxs[gi];
        auto& ins=ir[i];
        if(ins.op=="comment"&&isParallelComment(ins.arg1)){
            ParallelLoop pl=detectParallelLoop(i);
            if(pl.found){
                if(useCUDA&&isLargeLoop(pl.bound)){
                    out<<"    // CUDA kernel for loop bound="<<pl.bound<<"\n";
                } else {
                    out<<"\n    // auto-parallelised by BulkCompiler\n";
                    out<<"    #pragma omp parallel for schedule(static)\n";
                    out<<"    for(int "<<pl.idxVar<<"=0; "
                       <<pl.idxVar<<"<"<<pl.bound<<"; ++"<<pl.idxVar<<") {\n";
                    emitRange(out,pl.bodyStart,pl.bodyEnd,"        ",useCUDA,exeName,cuFile);
                    out<<"    }\n";
                }
                while(gi<globalIdxs.size()&&globalIdxs[gi]<=pl.lendIdx) ++gi;
                continue;
            }
        }
        if(ins.op=="func_begin"||ins.op=="func_end"||ins.op=="param"){++gi;continue;}
        if(ins.op=="push_arg"){pendingArgs.push_back(ins.arg1);++gi;continue;}
        if(ins.op=="call"){
            std::string al;
            for(size_t a=0;a<pendingArgs.size();++a){if(a) al+=", ";al+=pendingArgs[a];}
            pendingArgs.clear();
            if(ins.result.empty()) out<<"    "<<ins.arg1<<"("<<al<<");\n";
            else out<<"    "<<ins.result<<" = "<<ins.arg1<<"("<<al<<");\n";
            ++gi;continue;
        }
        std::string line=emitC(ins,"    ");
        if(!line.empty()) out<<line<<"\n";
        ++gi;
    }
    out<<"    return 0;\n}\n";
    out.close();

    std::cout<<"[Codegen] C file written to: "<<cFile<<"\n";

    std::string cmd="gcc -O2 -fopenmp "+cFile+" -o "+exeName+" 2>&1";
    std::cout<<"[Codegen] Compiling: "<<cmd<<"\n";
    int ret=system(cmd.c_str());
    if(ret==0) std::cout<<"[Codegen] Success! Run with: ./"<<exeName<<"\n";
    else std::cerr<<"[Codegen] gcc failed — see errors above\n";
}

// ── Assembly generation ───────────────────────────────────────────────────────
void generateAssembly(const std::string& cFile, const std::string& outFile,
                      const std::string& arch)
{
    std::string flag;
    if(arch=="x86")     flag="-m32";
    else if(arch=="x86_64") flag="";
    else if(arch=="arm")  flag="--target=arm-linux-gnueabi";
    else if(arch=="riscv") flag="-march=rv64gc -mabi=lp64d";

    std::string cmd="gcc -S -O2 "+flag+" "+cFile+" -o "+outFile+" 2>&1";
    std::cout<<"[ASM] Generating "<<arch<<" assembly: "<<cmd<<"\n";
    int r=system(cmd.c_str());
    if(r==0) std::cout<<"[ASM] Assembly written to: "<<outFile<<"\n";
    else std::cerr<<"[ASM] Failed\n";
}

// ── CFG dot file generation ───────────────────────────────────────────────────
void generateCFG(const std::string& dotFile)
{
    std::ofstream out(dotFile);
    if(!out){std::cerr<<"Error: cannot open "<<dotFile<<"\n";return;}

    out<<"digraph CFG {\n";
    out<<"  node [shape=box fontname=\"Courier\" fontsize=10];\n";

    // Split IR into basic blocks
    struct Block { std::string id; std::vector<std::string> instrs; std::vector<std::string> succs; };
    std::vector<Block> blocks;
    std::unordered_map<std::string,int> labelToBlock;

    // Determine leaders (first instr, targets of jumps, instr after jump)
    std::set<size_t> leaders;
    leaders.insert(0);
    for(size_t i=0;i<ir.size();++i){
        if(ir[i].op=="goto"||ir[i].op=="ifzero_goto"||ir[i].op=="return"){
            if(i+1<ir.size()) leaders.insert(i+1);
        }
        if(ir[i].op=="label") leaders.insert(i);
    }

    // Build blocks
    int bn=0;
    for(auto it=leaders.begin();it!=leaders.end();++it){
        Block b;
        b.id="B"+std::to_string(bn++);
        auto next=std::next(it);
        size_t end=(next!=leaders.end())?*next:ir.size();
        for(size_t j=*it;j<end;++j){
            if(ir[j].op=="label") labelToBlock[ir[j].arg1]=(int)blocks.size();
            std::string txt=ir[j].op;
            if(!ir[j].arg1.empty()) txt+=" "+ir[j].arg1;
            if(!ir[j].arg2.empty()) txt+=" "+ir[j].arg2;
            if(!ir[j].result.empty()) txt=" "+ir[j].result+"="+txt;
            b.instrs.push_back(txt);
        }
        blocks.push_back(b);
    }

    // Add edges
    for(size_t b=0;b<blocks.size();++b){
        // find last real instruction
        auto& blk=blocks[b];
        auto it=std::next(leaders.begin(),b);
        auto nxt=std::next(it);
        size_t end=(nxt!=leaders.end())?*nxt:ir.size();
        if(end==0) continue;
        auto& last=ir[end-1];
        if(last.op=="goto"){
            auto tit=labelToBlock.find(last.arg1);
            if(tit!=labelToBlock.end()) blk.succs.push_back("B"+std::to_string(tit->second));
        } else if(last.op=="ifzero_goto"){
            if(b+1<blocks.size()) blk.succs.push_back("B"+std::to_string(b+1));
            auto tit=labelToBlock.find(last.arg2);
            if(tit!=labelToBlock.end()) blk.succs.push_back("B"+std::to_string(tit->second));
        } else if(last.op!="return"){
            if(b+1<blocks.size()) blk.succs.push_back("B"+std::to_string(b+1));
        }
    }

    // Emit nodes
    for(auto& b:blocks){
        out<<"  "<<b.id<<" [label=\""<<b.id<<"\\n";
        for(auto& ins:b.instrs) {
            std::string esc=ins;
            for(char& c:esc) if(c=='"') c='\'';
            out<<esc<<"\\l";
        }
        out<<"\"];\n";
    }
    // Emit edges
    for(auto& b:blocks)
        for(auto& s:b.succs)
            out<<"  "<<b.id<<" -> "<<s<<";\n";

    out<<"}\n";
    out.close();
    std::cout<<"[CFG] Dot file written to: "<<dotFile<<"\n";
    // Try to render
    int r=system(("dot -Tpng "+dotFile+" -o "+dotFile+".png 2>/dev/null").c_str());
    if(r==0) std::cout<<"[CFG] PNG rendered to: "<<dotFile<<".png\n";
}

// ── AST dot file (pretty print) ───────────────────────────────────────────────
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
