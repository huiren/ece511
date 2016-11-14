/*
 * Copyright (c) 2014 The Regents of The University of Michigan
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
 *
 * Authors: Anthony Gutierrez
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "cpu/pred/fastPath.hh"

fastPathBP::fastPathBP(const fastPathBPParams *params)
    : BPredUnit(params),
      globalHistoryReg(params->numThreads, 0),
      globalHistoryBits(ceilLog2(WEIGHT_ENTRY_NUM)),
      globalPredictorSize(WEIGHT_ENTRY_NUM),
      globalCtrBits(params->globalCtrBits)
{
    
    if (!isPowerOf2(globalPredictorSize))
        fatal("Invalid global history predictor size.\n");

    
    // takenCounters.resize(globalPredictorSize);

    // for (int i = 0; i < globalPredictorSize; ++i) {
    //     takenCounters[i].setBits(globalCtrBits);
    // }
    SG = 0;
    G = 0;
    for(int i = 0; i < WEIGHT_ENTRY_NUM; i++) {
        SR[i] = 0;
        R[i] = 0;
        v[i] = 0;
        s_v[i] = 0;
        H[i] = true;
        for(int j = 0; j < WEIGHT_NUM; j++) {
            weights[i][j] = 0;
        }
    }

    historyRegisterMask = mask(globalHistoryBits);
    globalHistoryMask = globalPredictorSize - 1;

    takenThreshold = (ULL(1) << (globalCtrBits - 1)) - 1;
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
fastPathBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{                    
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->result = 1;
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
fastPathBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

/*
 * Here we lookup the actual branch prediction. We use the PC to
 * identify the bias of a particular branch, which is based on the
 * prediction in the choice array. A hash of the global history
 * register and a branch's PC is used to index into both the taken
 * and not-taken predictors, which both present a prediction. The
 * choice array's prediction is used to select between the two
 * direction predictors for the final branch prediction.
 */
bool
fastPathBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    unsigned globalHistoryIdx = branchAddr >> instShiftAmt;
                                // ^ globalHistoryReg[tid])
                                // & globalHistoryMask);

    // assert(globalHistoryIdx < globalPredictorSize);

    // bool takenGHBPrediction = takenCounters[globalHistoryIdx].read()
    //                           > takenThreshold;

    bool finalPrediction;

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    
    // finalPrediction = takenGHBPrediction;
    // unsigned GHR = globalHistoryReg[tid];
    unsigned entryIdx = globalHistoryIdx % WEIGHT_ENTRY_NUM;

    int result = weights[entryIdx][0] + SR[WEIGHT_NUM-1];

    // for(int i = 0; i < WEIGHT_NUM - 1; i++){
    //     result += weights[entryIdx][i + 1] * (((GHR >> i) & 1)==1?1:-1);
    // }
    if(result >= 0)
        finalPrediction = true;
    else
        finalPrediction = false;
    history->result = result;
    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, finalPrediction);
    //prediction is done
    //update partial sum
    for(int i = 1; i < WEIGHT_ENTRY_NUM; i++) {
        s_v[i] = s_v[i-1];
    }
    s_v[0] = entryIdx;

    for(int i = 0; i < WEIGHT_ENTRY_NUM; i++) {
        v[i] = s_v[i];
        H[i] = ((SG >> i) & 1)==1?true:false;
    }

    int SR_tmp[WEIGHT_ENTRY_NUM];
    for(int j = 1; j < WEIGHT_ENTRY_NUM; j++) {
        int k = WEIGHT_ENTRY_NUM - 1 - j;
        if(finalPrediction) {
            SR_tmp[k + 1] = SR[k] + weights[entryIdx][j]; 
        }
        else {
            SR_tmp[k + 1] = SR[k] - weights[entryIdx][j]; 
        }
    }
    for(int i = 0; i < WEIGHT_ENTRY_NUM; i++) {
        SR[i] = SR_tmp[i];
    }
    SR[0] = 0;
    SG = (SG << 1)|(finalPrediction?1:0);
    return finalPrediction;
}

void
fastPathBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

/* Only the selected direction predictor will be updated with the final
 * outcome; the status of the unselected one will not be altered. The choice
 * predictor is always updated with the branch outcome, except when the
 * choice is opposite to the branch outcome but the selected counter of
 * the direction predictors makes a correct final prediction.
 */
void
fastPathBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    if (bpHistory) {
        BPHistory *history = static_cast<BPHistory*>(bpHistory);

        unsigned globalHistoryIdx = branchAddr >> instShiftAmt;
                                    // ^ history->globalHistoryReg)
                                    // & globalHistoryMask);

        // assert(globalHistoryIdx < globalPredictorSize);
        // unsigned GHR = history->globalHistoryReg;
        unsigned result = history->result;
        bool finalPred = history->finalPred;
        // if the taken array's prediction was used, update it
        // if (taken) {
        //     // takenCounters[globalHistoryIdx].increment();
        // } else {
        //     takenCounters[globalHistoryIdx].decrement();
        // }
        unsigned entryIdx = globalHistoryIdx % WEIGHT_ENTRY_NUM;

        for(int i = 1; i < WEIGHT_ENTRY_NUM; i++) {
            int k = WEIGHT_ENTRY_NUM - 1 - i;
            if(taken) {
                R[k + 1] = R[k] + weights[entryIdx][i];
            }
            else {
                R[k + 1] = R[k] - weights[entryIdx][i];
            }
        }
        R[0] = 0;

        G = (G<<1)|(taken?1:0);
        for(int i = 1; i < WEIGHT_ENTRY_NUM; i++) {
            v[i] = v[i - 1];
        }
        v[0] = entryIdx;

        if(taken != finalPred) {
            for(int i = 0; i < WEIGHT_ENTRY_NUM; i++) {
                SR[i] = R[i];
                s_v[i] = v[i];
            }
            SG = G;
        }

        if(((taken == true) && (finalPred == false)) || ((taken == false) && (finalPred == true)) 
             || std::abs(result) <= THRESHOLD) {
            weights[entryIdx][0] = taken?1:-1 + weights[entryIdx][0];
            for(int i = 1; i < WEIGHT_NUM; i++) {
                int k = v[i];
                weights[k][i] += taken==H[i]?1:-1; 
            }
        }  

        if (squashed) {
            if (taken) {
                globalHistoryReg[tid] = (history->globalHistoryReg << 1) | 1;
            } else {
                globalHistoryReg[tid] = (history->globalHistoryReg << 1);
            }
            globalHistoryReg[tid] &= historyRegisterMask;
        } else {
            delete history;
        }
    }
}

void
fastPathBP::retireSquashed(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory*>(bp_history);
    delete history;
}

unsigned
fastPathBP::getGHR(ThreadID tid, void *bp_history) const
{
    return static_cast<BPHistory*>(bp_history)->globalHistoryReg;
}

void
fastPathBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}

fastPathBP*
fastPathBPParams::create()
{
    return new fastPathBP(this);
}
