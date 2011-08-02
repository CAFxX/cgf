#include "Utils.h"

Value* undef(const Type *t) {
  return UndefValue::get(t);
}

Value* undef(const Value *v) {
  return undef(v->getType());
}

bool isImmediatePredecessor(BasicBlock *pred, BasicBlock *bb) {
  TerminatorInst *t = pred->getTerminator();
  for (unsigned i=0, ie=t->getNumSuccessors(); i != ie; i++)
    if (t->getSuccessor(i) == bb)
      return true;
  return false;
}

const Type* getContainingIntTy(LLVMContext &c, uint64_t val) {
  for (unsigned n=0; n<sizeof(val)*8; n++)
    if (((uint64_t)1)<<n > val)
      return Type::getIntNTy(c, n+1);
  return Type::getInt64Ty(c);
}

//////////////////////////////////////////////////////////////////////
// Value propagation                                                //
//////////////////////////////////////////////////////////////////////

const char mdkValuePropagation[] = "VP";

unsigned numPHIs = 0;

bool Dominates(Instruction *v, Instruction *dominator) {
    BasicBlock *bb = v->getParent();
    assert(bb == dominator->getParent());
    bool dominatorFound = false;
    for (BasicBlock::iterator i=bb->begin(), ie=bb->end(); i != ie; i++)
        if (&*i == dominator)
            dominatorFound = true;
        else if (&*i == v)
            return dominatorFound;
    assert(!"unreacheable");        
}

Value* PropagateValue(Instruction *v, BasicBlock *from, std::map<BasicBlock*, Value*> &inc) {
    if (inc.count(from) > 0)
        return inc[from];

    // check if the value is defined in this basic block
    if (v->getParent() == from)
        return v;
    
    // recursively check if the value is defined in the predecessors
    bool found = false;
    unsigned preds = 0;
    Value *UV = undef(v);
    PHINode *phi = PHINode::Create(v->getType(), "", from->begin());
    Value *elts[] = { v };
    phi->setMetadata(mdkValuePropagation, MDNode::get(v->getContext(), elts));
    inc[from] = phi;
    std::vector<BasicBlock*> wlBB;
    for (pred_iterator i=pred_begin(from), ie=pred_end(from); i != ie; i++, preds++) {
        wlBB.push_back(*i);
    }
    for (std::vector<BasicBlock*>::iterator i = wlBB.begin(), ie = wlBB.end(); i != ie; i++) {
        Value *pv = PropagateValue(v, *i, inc);
        phi->addIncoming(pv, *i);
        if (pv != UV)
            found = true;
    }
    if (!found) {
        inc[from] = UV;
        phi->eraseFromParent();
        return UV;
    }
    
    numPHIs++;
    //statPHIsBrokenValues++;
    return phi;
}

std::map< Value*, std::map<BasicBlock*, Value*> > map;

Value *PropagateValue(Value *v, BasicBlock *from) {
    if (isa<Constant>(v) || isa<BasicBlock>(v) || BlockAddress::classof(v))
        return v;

    Argument *arg = dyn_cast<Argument>(v);
    if (arg && arg->getParent() == from->getParent())
        return v;
    else if (arg)
        assert(!"Can't propagate argument from a different function!");

    Instruction *inst = dyn_cast<Instruction>(v);
    assert(inst);

    Value *ret = PropagateValue(inst, from, map[v]);

    return ret != undef(v) ? ret : NULL;
}

Value* GetOriginalValue(Value *i) {
  Instruction *j = dyn_cast<Instruction>(i);
  if (!j) return i;
  MDNode *md = j->getMetadata(mdkValuePropagation);
  return md ? GetOriginalValue(md->getOperand(0)) : i;
}

unsigned PropagateValues(BasicBlock *bb) {
    unsigned count = 0;
    std::set<Value*> bbVals;
    std::set<Instruction*> wlVals;

    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
        if (!isa<PHINode>(&*i)) {
            wlVals.insert(&*i);
        }
    }
    
    for (std::set<Instruction*>::iterator i = wlVals.begin(), ie = wlVals.end(); i != ie; ++i) {    
        for (User::op_iterator j = (*i)->op_begin(), je = (*i)->op_end(); j != je; ++j) {
            if (bbVals.count(j->get()) == 0) {
                Value *pVal = PropagateValue(GetOriginalValue(j->get()), bb);
                assert(pVal);
                j->set(pVal);
                count++;
            }                
        }
        bbVals.insert(*i);
    }

    return count;
}

