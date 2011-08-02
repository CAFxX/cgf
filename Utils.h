#include "Includes.h"

#define TRACE() TRACEMSG("")
#define TRACEMSG(msg) DEBUG(errs() << __PRETTY_FUNCTION__ << ":" << __LINE__ << " " << (msg) << "\n")

//////////////////////////////////////////////////////////////////////
// Misc helpers                                                     //
//////////////////////////////////////////////////////////////////////

Value* undef(const Type *t);
Value* undef(const Value *v);

bool isImmediatePredecessor(BasicBlock *pred, BasicBlock *bb);

//////////////////////////////////////////////////////////////////////
// Value propagation                                                //
//////////////////////////////////////////////////////////////////////

Value *PropagateValue(Value *v, BasicBlock *from);
unsigned PropagateValues(BasicBlock *bb);

