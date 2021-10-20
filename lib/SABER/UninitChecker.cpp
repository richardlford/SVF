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
    return !isa<StoreSVFGNode>(node) && !isa<AllocUninitSVFGNode>(node);
}
void UninitChecker::reportBug(ProgSlice* slice)
{
    const SVFGNode* src = slice->getSource();
    const auto* aunode = cast<AllocUninitSVFGNode>(src);
    int num_sinks = slice->getSinks().size();
    if (num_sinks > 0)
    {
        const llvm::Value* val = aunode->getValue();
        const MemSSA::ALLOCCHI* mychi = aunode->getAllocChi();
        const BasicBlock* bb = mychi->getBasicBlock();
        const llvm::Function* fun = bb->getParent();

        SVFUtil::errs() << bugMsg2("\t Uninitialized :")
                        <<  " variable use in function " << fun->getName() << " at : ("
                        << getSourceLoc(val) << ")\n";
        SVFUtil::errs() << "\t\tAssociated value is " << *val << "\n";
        SVFUtil::errs() << "\t\t uninitialized path: \n" << slice->evalFinalCond() << "\n";
        SVFUtil::errs() << "\t\t Users of value:\n";
        int user_num = 0;
        for(auto node : slice->getSinks())
        {
            user_num++;
            llvm::errs() << "\t\t\tUser#" << user_num << ": " << *node << "\n";
        }
    }
}

/// Return true to skip following.
bool UninitChecker::FWProcessCurNode(const SrcSnkDDA::DPIm &item) {
    const SVFGNode* node = getNode(item.getCurNodeID());
    if(isSink(node))
    {
        addSinkToCurSlice(node);
        _curSlice->setPartialReachable();
        return true;
    }
    else
        addToCurForwardSlice(node);
    // We only keep following if it is an AllocUninitSVFGNode,
    // If not, it is a store and we want to stop.
    return !SVFUtil::isa<AllocUninitSVFGNode>(node);
}

bool UninitChecker::BWProcessCurNode(const SrcSnkDDA::DPIm &item) {
    const SVFGNode* node = getNode(item.getCurNodeID());
    if(isInCurForwardSlice(node))
    {
        addToCurBackwardSlice(node);
    }
    return false;
}

