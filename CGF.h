#ifndef __CGF__
#define __CGF__ 1

struct CGFFunction;
struct CGFCallSite;

struct CGF {
    ModulePass *p;
    Module *m;
    Function *dispatch;
    Argument *dispatchArgs;
    Argument *dispatchRetVal;
    BasicBlock *unreachable;
    const Type *opaqueTy;

    typedef std::map<const Function*, CGFFunction*> funcMap;    
    typedef std::map<const CallInst*, CGFCallSite*> callsiteMap;    
    funcMap funcs; // map Function to CGFFunctions
    callsiteMap calls; // map CallInst to CGFCallInst

    SwitchInst *outerDispatcher;

    CGF(ModulePass *pass, Module *module);
    void run();
    void registerFunction(Function *f);
    void registerCallsite(CallInst *s);
    void createDispatchFunction();
    void fixDummyPhis();
    void fixBrokenValues();
};

struct CGFCallSite {
    const CGF *c;
    CallInst *s;
    
    bool prepared;
    bool flattened;

    CGFFunction *Parent;
    CGFFunction *Callee;
    std::vector<Value*> Args;
    Value *RetVal;
    std::set<Value*> LiveVals;
    std::set<BasicBlock*> Successors;
    BasicBlock *CallBB;
    BasicBlock *RetBB;

    CGFCallSite(CGF *cgf, CallInst *cs);
    void prepare();
    void flatten();
    void splitRetBB();
};

struct CGFFunction {
    const CGF *c;
    Function *f;

    bool prepared;
    bool flattened;

    std::vector<CGFCallSite*> callers;
    std::vector<CGFCallSite*> callees;
    BasicBlock *outerDispatch; // when called externally
    BasicBlock *innerDispatch; // contains the phi nodes
    BasicBlock *returnDispatch; // wraps the return value for returns to external callers
    BasicBlock *entry; // actual entry block
    BasicBlock *exit; // actual exit block
    ConstantInt *entryBB; // switch index for outerdispatcher

    StructType *argsTy; // parameter unwrapping in outerDispatch
    const Type *retValTy; // return value wrapping
    std::vector<PHINode*> argphis; // arg phis in innerDispatch
    PHINode *retphi; // phi holding the address of the bb to return to
    Value *retVal; // return value
    SwitchInst *retinst; // return instruction of the function

    std::map<BasicBlock*, PHINode*> lvMap; // map basicblock->phi
    std::map<BasicBlock*, CGFCallSite*> csMap; // map basicblock->cgfcallsite
    ValueToValueMapTy vmap; // valuemap from original to cloned function

    CGFFunction(CGF *cgf, Function *func);
    void prepare();
    void flatten();
    void registerCaller(CGFCallSite *s);
    void registerCallee(CGFCallSite *s);
    void createForwardingWrapper(bool replace);
    void fixDummyPhis();

    bool hasArguments();
    bool hasReturnValue();
};

#endif

