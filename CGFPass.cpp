#define DEBUG_TYPE "cgf"

#include "Includes.h"
#include "Utils.h"

using namespace llvm;

STATISTIC(statCanonicalizedCallsites, "Number of canonicalized callsites");
STATISTIC(statFixedBrokenValues, "Number of fixed broken values"); 
//STATISTIC(statPHIsBrokenValues, "Number of PHIs for broken values"); 

namespace {

#include "CGF.h"
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
      CGF *C = new CGF(this, &m);
      C->run();
      return true;
    }
  };
}

char CGFPass::ID = 0;
static RegisterPass<CGFPass>
Y("cgf", "Call Graph Flattening");

