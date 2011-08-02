#include "Includes.h"

#define TRACE() TRACEMSG("")
#define TRACEMSG(msg) DEBUG(errs() << __PRETTY_FUNCTION__ << ":" << __LINE__ << " " << (msg) << "\n")

//////////////////////////////////////////////////////////////////////
// Misc helpers                                                     //
//////////////////////////////////////////////////////////////////////

Value* undef(const Type *t);
Value* undef(const Value *v);

bool isImmediatePredecessor(BasicBlock *pred, BasicBlock *bb);

const Type* getContainingIntTy(LLVMContext &c, uint64_t val);

//////////////////////////////////////////////////////////////////////
// Value propagation                                                //
//////////////////////////////////////////////////////////////////////

Value *PropagateValue(Value *v, BasicBlock *from);
unsigned PropagateValues(BasicBlock *bb);

