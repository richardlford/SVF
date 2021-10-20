//===- UninitChecker.cpp -- Detecting uninitialized variables ------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * UninitChecker.cpp
 *
 *  Created on: Sep 28, 2021
 *      Author: Richard L. Ford
 */

#include "SABER/UninitChecker.h"
#include "SVF-FE/PAGBuilder.h"

using namespace SVF;
using namespace SVFUtil;

/// Initialize analysis
void UninitChecker::initialize(SVFModule* module)
{
    PAGBuilder builder;
    PAG* pag = builder.build(module);

    AndersenWaveDiff* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    svfg =  memSSA.buildFullSVFGWithoutOPT(ander);
    setGraph(memSSA.getSVFG());
    ptaCallGraph = ander->getPTACallGraph();
    //AndersenWaveDiff::releaseAndersenWaveDiff();
    /// allocate control-flow graph branch conditions
    getPathAllocator()->allocate(getPAG()->getModule());

    initSrcs();
    initSnks();
}

/// Initialize sources and sinks
void UninitChecker::initSrcs()
{
    auto eit = svfg->end();
    for (auto it = svfg->begin(); it != eit; it++) {
        auto entry = *it;
        auto nodeId = entry.first;
        auto nodePtr = entry.second;
        if (auto* aunode = dyn_cast<AllocUninitSVFGNode>(nodePtr))
        {
            this->addToSources(aunode);
        }
    }
}

void UninitChecker::initSnks()
{

}

bool UninitChecker::isSink(const SVFGNode* node) const {
    return !isa<StoreSVFGNode>(node);
}
void UninitChecker::reportBug(ProgSlice* slice)
{

    if(!slice->isSatisfiableForPairs())
    {
        const SVFGNode* src = slice->getSource();
        if (const AllocUninitSVFGNode* aunode = dyn_cast<AllocUninitSVFGNode>(src))
        {
            const llvm::Value* val = aunode->getValue();
            SVFUtil::errs() << bugMsg2("\t Uninitialized :") <<  " variable use at : ("
                            << getSourceLoc(val) << ")\n";
            SVFUtil::errs() << "\t\t uninitialized path: \n" << slice->evalFinalCond() << "\n";
            slice->annotatePaths();
        }
    }
}

