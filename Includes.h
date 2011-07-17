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
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/ADT/ValueMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Dominators.h"
#include <map>
#include <vector>
#include <set>

using namespace llvm;

