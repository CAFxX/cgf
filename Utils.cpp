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

void PropagateValue(Value *v, BasicBlock *from, std::map<BasicBlock*, Value*> &inc) {
    // check if the value is defined in this basic block
    for (BasicBlock::iterator i=from->begin(), ie=from->end(); i != ie; i++) {
        if (&*i == v) {
            inc[from] = v;
            return;
        }
    }
    
    // check if the value is defined in the predecessors
    Value *undef = UndefValue::get(v->getType());
    bool found = false;
    for (pred_iterator i=pred_begin(from), ie=pred_end(from); i != ie; i++) {
        BasicBlock *bb = *i;
        TerminatorInst *ti = bb->getTerminator();
        for (unsigned j=0, je=ti->getNumSuccessors(); j < je; j++) {
            if (ti->getSuccessor(j) == from) {
                if (inc.count(bb) == 0) {
                    inc[bb] = undef; // temporary value to avoid infinite recursion
                    PropagateValue(v, bb, inc);
                }
                if (inc[bb] != undef) {
                    found = true;
                }
                break;
            }
        }
    }
    if (found) {
        PHINode *phi = PHINode::Create(v->getType(), v->getName()+"__", from->begin());
        inc[from] = phi;
        for (pred_iterator i=pred_begin(from), ie=pred_end(from); i != ie; i++) {
            phi->addIncoming(inc[*i], *i);
        }
        return;
    }
    
    // the value is not defined in the basicblock or its predecessors
    inc[from] = undef;
}

Value *PropagateValue(Value *v, BasicBlock *from) {
    static std::map< Value*, std::map<BasicBlock*, Value*> > map;

    if (isa<Constant>(v) || isa<BasicBlock>(v) || BlockAddress::classof(v))
        return v;

    for (Function::arg_iterator i = from->getParent()->arg_begin(), ie = from->getParent()->arg_end(); i != ie; i++)
        if (v == &*i)
            return v;

    PropagateValue(v, from, map[v]);
    Value *ret = map[v][from];
    map.clear();
    return ret != UndefValue::get(v->getType()) ? ret : NULL;
}

