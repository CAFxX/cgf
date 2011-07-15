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


