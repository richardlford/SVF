//===- UninitChecker.h -- Detecting uninitialized variables ---------------------//
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
 * UninitChecker.h
 *
 *  Created on: Sep 28, 2021
 *      Author: Richard L. Ford
 */

#ifndef UNINITCHECKER_H_
#define UNINITCHECKER_H_

#include "SABER/LeakChecker.h"

namespace SVF
{

/*!
 * Check for use of uninitialized variables.
 */

class UninitChecker : public LeakChecker
{

public:
    /// Constructor
    UninitChecker(): LeakChecker()
    {
    }

    /// Destructor
    virtual ~UninitChecker()
    {
    }

    /// We start from here
    virtual bool runOnModule(SVFModule* module)
    {
        /// start analysis
        analyze(module);
        return false;
    }

    /// Initialize analysis
    void initialize(SVFModule* module);

    /// Initialize sources and sinks
    virtual void initSrcs() override;
    virtual void initSnks() override;

    virtual bool isSink(const SVFGNode* node) const override;

    /// Report file/close bugs
    void reportBug(ProgSlice* slice);
};

} // End namespace SVF

#endif /* UNINITCHECKER_H_ */
