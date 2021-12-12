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
      choicePredictorSize(params.choicePredictorSize),
      choiceCtrBits(params.choiceCtrBits),
      choiceCtrs(choicePredictorSize, SatCounter8(choiceCtrBits, ((1ULL << (params.choiceCtrBits - 1))- 1)))
{

    // Set up choiceHistoryMask
    // this is equivalent to mask(log2(choicePredictorSize)
    choiceHistoryMask = choicePredictorSize - 1;

    // Set thresholds for the three predictors' counters
    // This is equivalent to (2^(Ctr))/2 - 1
    choiceThreshold = (1ULL << (choiceCtrBits - 1)) - 1;

}


void
Tournament2BP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    //unsigned local_history_idx = calcLocHistIdx(branch_addr);
    BPHistory *history = static_cast<BPHistory *>(bp_history);
    void *tage_history = history->BPTage;
    void *mpp_history = history->BPMPP;
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
            return tage_prediction;
            }
    else {
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
    history->mpp_predicted = true;
    history->tage_predicted = true;
    history->uncond = true;
    bp_history = (void *)history;
   
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

    if (history->mpp_predicted != history->tage_predicted && history->uncond==false){
        unsigned choice_predictor_idx = ((branch_addr >> instShiftAmt) & choiceHistoryMask);

        if (taken == history->tage_predicted) {
            choiceCtrs[choice_predictor_idx]++;
        }
        else if(taken == history->mpp_predicted) {
            choiceCtrs[choice_predictor_idx]--;
        }
    }
    
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
    delete(history);
}

#ifdef DEBUG
int
Tournament2BP::BPHistory::newCount = 0;
#endif

} // namespace branch_prediction
} // namespace gem5
