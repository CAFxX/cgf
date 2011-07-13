//===- cfgf.cpp - CFG Flattening ------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cgf"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/User.h"
#include "llvm/GlobalValue.h" // LinkageTypes
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h" // SplitBlock
#include "llvm/Transforms/Utils/ValueMapper.h" // ValueToValueMapTy
#include "llvm/Transforms/Utils/Cloning.h" // ClonedCodeInfo
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Support/CallSite.h" // CallInst
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Support/Debug.h"
#include <map>
#include <vector>
#include <set>

using namespace llvm;

STATISTIC(statCanonicalizedCallsites, "Number of canonicalized callsites");
STATISTIC(statFixedBrokenValues, "Number of fixed broken values"); 

#define TRACE() TRACEMSG("")
#define TRACEMSG(msg) DEBUG(errs() << __PRETTY_FUNCTION__ << ":" << __LINE__ << " " << (msg) << "\n")

namespace {

#include "CGF.h"
#include "go_fetch.cppi"
#include "CGF.cppi"
#include "CGFCallsite.cppi"
#include "CGFFunction.cppi"

  struct CGFPass : public ModulePass {
    static char ID; // Pass identification, replacement for typeid
    CGFPass() : ModulePass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequiredID(BreakCriticalEdgesID);
      AU.addRequired<UnifyFunctionExitNodes>();
      AU.addRequired<DominatorTree>();
    }

    virtual bool runOnModule(Module &m) {
      TRACE();
      CGF *C = new CGF(this, &m);
      C->run();
      return true;
    }

  };
  
}

char CGFPass::ID = 0;
static RegisterPass<CGFPass>
Y("cgf", "Call Graph Flattening");

