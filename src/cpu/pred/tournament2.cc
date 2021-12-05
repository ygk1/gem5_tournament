/*
 * Copyright (c) 2011, 2014 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "cpu/pred/tournament2.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "cpu/pred/multiperspective_perceptron_8KB.hh"
#include "cpu/pred/tage.hh"
#include "cpu/pred/tage_base.hh"
#include "debug/Tage.hh"

namespace gem5
{

namespace branch_prediction
{

Tournament2BP::Tournament2BP(const Tournament2BPParams &params)
    : BPredUnit(params),
      tage_pred(params.tage),
      prec_pred(params.perceptron),
      localPredictorSize(params.localPredictorSize),
      localCtrBits(params.localCtrBits),
      localCtrs(localPredictorSize, SatCounter8(localCtrBits)),
      localHistoryTableSize(params.localHistoryTableSize),
      localHistoryBits(ceilLog2(params.localPredictorSize)),
      globalPredictorSize(params.globalPredictorSize),
      globalCtrBits(params.globalCtrBits),
      globalCtrs(globalPredictorSize, SatCounter8(globalCtrBits)),
      globalHistory(params.numThreads, 0),
      globalHistoryBits(
          ceilLog2(params.globalPredictorSize) >
          ceilLog2(params.choicePredictorSize) ?
          ceilLog2(params.globalPredictorSize) :
          ceilLog2(params.choicePredictorSize)),
      choicePredictorSize(params.choicePredictorSize),
      choiceCtrBits(params.choiceCtrBits),
      choiceCtrs(choicePredictorSize, SatCounter8(choiceCtrBits))
{
    if (!isPowerOf2(localPredictorSize)) {
        fatal("Invalid local predictor size!\n");
    }

    if (!isPowerOf2(globalPredictorSize)) {
        fatal("Invalid global predictor size!\n");
    }

    localPredictorMask = mask(localHistoryBits);

    if (!isPowerOf2(localHistoryTableSize)) {
        fatal("Invalid local history table size!\n");
    }

    //Setup the history table for the local table
    localHistoryTable.resize(localHistoryTableSize);

    for (int i = 0; i < localHistoryTableSize; ++i)
        localHistoryTable[i] = 0;

    // Set up the global history mask
    // this is equivalent to mask(log2(globalPredictorSize)
    globalHistoryMask = globalPredictorSize - 1;

    if (!isPowerOf2(choicePredictorSize)) {
        fatal("Invalid choice predictor size!\n");
    }

    // Set up choiceHistoryMask
    // this is equivalent to mask(log2(choicePredictorSize)
    choiceHistoryMask = choicePredictorSize - 1;

    //Set up historyRegisterMask
    historyRegisterMask = mask(globalHistoryBits);

    //Check that predictors don't use more bits than they have available
    if (globalHistoryMask > historyRegisterMask) {
        fatal("Global predictor too large for global history bits!\n");
    }
    if (choiceHistoryMask > historyRegisterMask) {
        fatal("Choice predictor too large for global history bits!\n");
    }

    if (globalHistoryMask < historyRegisterMask &&
        choiceHistoryMask < historyRegisterMask) {
        inform("More global history bits than required by predictors\n");
    }

    // Set thresholds for the three predictors' counters
    // This is equivalent to (2^(Ctr))/2 - 1
    localThreshold  = (1ULL << (localCtrBits  - 1)) - 1;
    globalThreshold = (1ULL << (globalCtrBits - 1)) - 1;
    choiceThreshold = (1ULL << (choiceCtrBits - 1)) - 1;
    prediction = false;

}



inline
unsigned
Tournament2BP::calcLocHistIdx(Addr &branch_addr)
{
    // Get low order bits after removing instruction offset.
    return (branch_addr >> instShiftAmt) & (localHistoryTableSize - 1);
}

inline
void
Tournament2BP::updateGlobalHistTaken(ThreadID tid)
{
    globalHistory[tid] = (globalHistory[tid] << 1) | 1;
    globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

inline
void
Tournament2BP::updateGlobalHistNotTaken(ThreadID tid)
{
    globalHistory[tid] = (globalHistory[tid] << 1);
    globalHistory[tid] = globalHistory[tid] & historyRegisterMask;
}

inline
void
Tournament2BP::updateLocalHistTaken(unsigned local_history_idx)
{
    localHistoryTable[local_history_idx] =
        (localHistoryTable[local_history_idx] << 1) | 1;
}

inline
void
Tournament2BP::updateLocalHistNotTaken(unsigned local_history_idx)
{
    localHistoryTable[local_history_idx] =
        (localHistoryTable[local_history_idx] << 1);
}


void
Tournament2BP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    //unsigned local_history_idx = calcLocHistIdx(branch_addr);
    BPHistory *history = static_cast<BPHistory *>(bp_history);
    void *tage_history = history->BPTage;
    void *mpp_history = history->BPMPP;
    //Update Global History to Not Taken (clear LSB)
    //globalHistory[tid] &= (historyRegisterMask & ~1ULL);
    //Update Local History to Not Taken
    //localHistoryTable[local_history_idx] =
    //   localHistoryTable[local_history_idx] & (localPredictorMask & ~1ULL);
    tage_pred->btbUpdate(tid, branch_addr, tage_history);
    prec_pred->btbUpdate(tid, branch_addr, mpp_history);
}

bool
Tournament2BP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    //bool taken_prediction=true;
    bool tage_prediction;

    bool multilayer_preceptron_prediction;
    bool choice_prediction;
    BPHistory *history = new BPHistory;
    bp_history = history;
    void *history_tage;
    void *history_MPP;



    tage_prediction = tage_pred->lookup(tid, branch_addr, history_tage);
    multilayer_preceptron_prediction = prec_pred->lookup(tid, branch_addr,
    history_MPP);

    history->BPTage = static_cast<TAGE::TageBranchInfo*>(history_tage);
    history->BPMPP = static_cast<MultiperspectivePerceptron8KB::MPPBranchInfo*>
    (history_MPP);
    bp_history = (void *)history;
    //Lookup in the local predictor to get its branch prediction


    //Lookup in the choice predictor to see which one to use
    choice_prediction = choiceThreshold <
      choiceCtrs[(branch_addr >> instShiftAmt) & choiceHistoryMask];

    // Create BPHistory and pass it back to be recorded.


    //tage_prediction = true;
    // Speculative update of the global history and the
    // selected local history.
    history->tage_predicted = tage_prediction;
    history->mpp_predicted = multilayer_preceptron_prediction;
    if (choice_prediction) {
            updateGlobalHistTaken(tid);
            prediction = tage_prediction;
            
            return tage_prediction;
    } else {
            updateGlobalHistNotTaken(tid);
            prediction = multilayer_preceptron_prediction;
            return multilayer_preceptron_prediction;
        }
}

void
Tournament2BP::uncondBranch(ThreadID tid, Addr pc, void * &bp_history)
{
    
    // Create BPHistory and pass it back to be recorded.
    BPHistory *history = new BPHistory;
    void *history_tage;
    void *history_MPP;
    tage_pred->uncondBranch(tid, pc,  history_tage);
    prec_pred->uncondBranch(tid, pc, history_MPP );
    history->BPTage = static_cast<TAGE::TageBranchInfo*>(history_tage);
    history->BPMPP = static_cast<MultiperspectivePerceptron8KB::MPPBranchInfo*>
     (history_MPP);
    bp_history = (void *)history;
    updateGlobalHistTaken(tid);
}

void
Tournament2BP::update(ThreadID tid, Addr branch_addr, bool taken,
                     void *bp_history, bool squashed,
                     const StaticInstPtr & inst, Addr corrTarget)
{
    assert(bp_history);
    BPHistory *history = static_cast<BPHistory *>(bp_history);
    void *tage_history = history->BPTage;
    void *mpp_history = history->BPMPP;
    tage_pred->update(tid, branch_addr, taken, tage_history,
        squashed, inst, corrTarget);
    prec_pred->update(tid, branch_addr, taken, mpp_history,
        squashed, inst, corrTarget);
   
    if (squashed) {
        // Global history restore and update
        return ;
    }

    // Unconditional branches do not use local history.

    // If this is a misprediction, restore the speculatively
    // updated state (global history register and local history)
    // and update again.


    // Update the choice predictor to tell it which one was correct if
    // there was a prediction.

        // If the local prediction matches the actual outcome,
        // decrement the counter. Otherwise increment the
        // counter.
    unsigned choice_predictor_idx = (branch_addr & choiceHistoryMask);
    if (taken == history->tage_predicted) {
        choiceCtrs[choice_predictor_idx]++;
    }
    else if(taken == history->mpp_predicted) {
        choiceCtrs[choice_predictor_idx]--;
    }


    // Update the counters with the proper
    // resolution of the branch. Histories are updated
    // speculatively, restored upon squash() calls, and
    // recomputed upon update(squash = true) calls,
    // so they do not need to be updated.
    delete(history);
    // We're done with this history, now delete it.

}

void
Tournament2BP::squash(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory *>(bp_history);
    void *tage_history = history->BPTage;
    void *mpp_history = history->BPMPP;
    tage_pred->squash(tid, tage_history);
    prec_pred->squash(tid, mpp_history);
}

#ifdef DEBUG
int
Tournament2BP::BPHistory::newCount = 0;
#endif

} // namespace branch_prediction
} // namespace gem5
