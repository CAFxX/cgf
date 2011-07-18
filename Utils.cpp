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

//////////////////////////////////////////////////////////////////////
// Dummy Phis                                                       //
//////////////////////////////////////////////////////////////////////

const char mdkDummyPhi[] = "dummyphi";

PHINode* createDummyPhi(BasicBlock *BB, Value *v, BasicBlock *from) {
  Value *vOrig = v;
  if (PHINode *vphi = dyn_cast<PHINode>(v))
    if (MDNode *md = vphi->getMetadata(mdkDummyPhi))
      vOrig = md->getOperand(0);
  PHINode *phi = PHINode::Create(v->getType(), "cgf.dummyphi."+vOrig->getName(), BB);
  phi->addIncoming(v, from);
  Value *elts[] = { vOrig };
  phi->setMetadata(mdkDummyPhi, MDNode::get(v->getContext(), elts));
  return phi;
}

bool fixDummyPhi(PHINode *phi) {
  if (phi == NULL)
    return false;
  if (!phi->getMetadata(mdkDummyPhi))
    return false;
  BasicBlock *bb = phi->getParent();
  for (pred_iterator i = pred_begin(bb), ie = pred_end(bb); i != ie; i++)
    if (phi->getBasicBlockIndex(*i) < 0 && isImmediatePredecessor(*i, bb))
      phi->addIncoming(undef(phi), *i);
  return true;
}

bool fixDummyPhi(Value *v) {
  return fixDummyPhi(dyn_cast<PHINode>(v));
}

unsigned fixDummyPhis(Function *f) {
  unsigned numDummyPhis = 0;
  for (inst_iterator i = inst_begin(f), ie = inst_end(f); i != ie; i++)
    if (fixDummyPhi(&*i))
      numDummyPhis++;
  return numDummyPhis;
}

//////////////////////////////////////////////////////////////////////
// Value propagation                                                //
//////////////////////////////////////////////////////////////////////

const char mdkValuePropagation[] = "valprop";

unsigned numPHIs = 0;

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
    PHINode *phi = PHINode::Create(v->getType(), "" /*v->getName()+"__"*/, from->begin());
    Value *elts[] = { v };
    phi->setMetadata(mdkValuePropagation, MDNode::get(v->getContext(), elts));
    inc[from] = phi;
    for (pred_iterator i=pred_begin(from), ie=pred_end(from); i != ie; i++, preds++) {
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
    if (preds == 1) {
        Value *pv = phi->getIncomingValue(0);
        phi->replaceAllUsesWith(pv);
        phi->eraseFromParent();
        inc.erase(from);
        return pv;
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

unsigned PropagateValues(BasicBlock *bb) {
    TRACE();
    unsigned count = 0;
    std::set<Value*> bbVals;
    for (BasicBlock::iterator i = bb->begin(), ie = bb->end(); i != ie; ++i) {
        if (!i->getMetadata(mdkValuePropagation)) {
            for (User::op_iterator j = i->op_begin(), je = i->op_end(); j != je; ++j) {
                if (bbVals.count(j->get()) == 0) {
                    Value *pVal = PropagateValue(j->get(), bb);
                    assert(pVal);
                    j->set(pVal);
                    count++;
                }                
            }
        }
        bbVals.insert(i);
    }
    return count;
}

