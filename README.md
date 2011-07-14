# Call-graph flattening (CGF) transform for the LLVM compiler framework

This LLVM ModulePass performs call-graph flattening, a IR transformation that gets rid of "functions" and merges all code together. This removes the distinction between IPO and non-IPO transforms and allows other transforms to perform better and more thorough optimizations.

This pilot implementation was written as part of my thesis [Compiler optimizations based on call-graph flattening](http://cafxx.strayorange.com/app/cv/addendum/thesis/ferraris_compiler_optimizations_call_graph_flattening.pdf). It is not yet useful for real-world code, mostly because other parts of LLVM (e.g. register allocator, spiller) have to be improved to make the most out of the possibilities empowered by CGF.


