#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "nodetools.h"

// returns a transformed AST
void transform(const AST& input, const NodeID& id, AST& output);

// losslessly folds constant subtrees
void foldConstants(const AST& input, const NodeID& id, AST& output);

// turns all subtraction into addition of negation 
void eliminateSubtraction(const AST& input, const NodeID& id, AST& output);

#endif