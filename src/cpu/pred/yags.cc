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
#include "cpu/pred/yags.hh"

YagsBP::YagsBP(const YagsBPParams *params)
    : BPredUnit(params),
      globalHistoryReg(params->numThreads, 0),
      globalHistoryBits(ceilLog2(params->globalPredictorSize)),
      choicePredictorSize(params->choicePredictorSize),
      choiceCtrBits(params->choiceCtrBits),
      globalPredictorSize(params->globalPredictorSize),
      globalCtrBits(params->globalCtrBits)
{
    if (!isPowerOf2(choicePredictorSize))
        fatal("Invalid choice predictor size.\n");
    if (!isPowerOf2(globalPredictorSize))
        fatal("Invalid global history predictor size.\n");

    choiceCounters.resize(choicePredictorSize);
    takenCounters.resize(globalPredictorSize);
    notTakenCounters.resize(globalPredictorSize);
    takenLRU.resize(globalPredictorSize/2);    
    notTakenLRU.resize(globalPredictorSize/2);
    takenTags.resize(globalPredictorSize);
    notTakenTags.resize(globalPredictorSize);

    for (int i = 0; i < choicePredictorSize; ++i) {
        choiceCounters[i].setBits(choiceCtrBits);
    }
    for (int i = 0; i < globalPredictorSize; ++i) {
        takenCounters[i].setBits(globalCtrBits);
        notTakenCounters[i].setBits(globalCtrBits);
        takenTags[i] = 0;
        notTakenTags[i] = 0;
    }
    for (int i = 0; i < globalPredictorSize / 2; ++i) {
        takenLRU[i] = true;
        notTakenLRU[i] = true;
    }

    historyRegisterMask = mask(globalHistoryBits);
    choiceHistoryMask = choicePredictorSize - 1;
    globalHistoryMask = globalPredictorSize - 1;

    choiceThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    takenThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    notTakenThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
}

/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
YagsBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->takenUsed = 0;
    history->choicePred = true;
    history->takenPred = true;
    history->notTakenPred = true;
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
YagsBP::squash(ThreadID tid, void *bpHistory)
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
YagsBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    unsigned choiceHistoryIdx = ((branchAddr >> instShiftAmt)
                                & choiceHistoryMask);
    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ globalHistoryReg[tid])
                                & globalHistoryMask);
    //1024 different sets instead of 2048
    unsigned LRUIdx = (((branchAddr >> (instShiftAmt + 1))
                                ^ globalHistoryReg[tid])
                                & (globalHistoryMask >> 1));

    //10 bits index
    Addr tag = branchAddr >> (instShiftAmt + 10);

    assert(choiceHistoryIdx < choicePredictorSize);
    assert(globalHistoryIdx < globalPredictorSize);
    assert(LRUIdx < globalPredictorSize / 2);

    bool choicePrediction = choiceCounters[choiceHistoryIdx].read()
                            > choiceThreshold;
    bool takenGHBPrediction = takenCounters[globalHistoryIdx].read()
                              > takenThreshold;
    bool notTakenGHBPrediction = notTakenCounters[globalHistoryIdx].read()
                                 > notTakenThreshold;
    bool finalPrediction;

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];

    history->choicePred = choicePrediction;
    history->takenPred = takenGHBPrediction;
    history->notTakenPred = notTakenGHBPrediction;
    /*history->takenUsed = choicePrediction;

    if (choicePrediction) {
        finalPrediction = takenGHBPrediction;
    } else {
        finalPrediction = notTakenGHBPrediction;
    }

    history->finalPred = finalPrediction;*/
    if(choicePrediction) {
        if(hitInNotTaken(tag, LRUIdx)) {
            finalPrediction = notTakenGHBPrediction;
            history->takenUsed = 2;
        }
        else {
            finalPrediction = choicePrediction;
            history->takenUsed = 0;
        }
    }
    else {
        if(hitInTaken(tag, LRUIdx)) {
            finalPrediction = takenGHBPrediction;
            history->takenUsed = 1;
        }
        else {
            finalPrediction = choicePrediction;
            history->takenUsed = 0;
        }
    }

    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, finalPrediction);

    return finalPrediction;
}

void
YagsBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
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
YagsBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed)
{
    if (bpHistory) {
        BPHistory *history = static_cast<BPHistory*>(bpHistory);

        unsigned choiceHistoryIdx = ((branchAddr >> instShiftAmt)
                                    & choiceHistoryMask);
        unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                    ^ history->globalHistoryReg)
                                    & globalHistoryMask);
        unsigned LRUIdx = (((branchAddr >> (instShiftAmt + 1))
                                ^ history->globalHistoryReg)
                                    & (globalHistoryMask >> 1));


        assert(choiceHistoryIdx < choicePredictorSize);
        assert(globalHistoryIdx < globalPredictorSize);
        assert(LRUIdx < globalPredictorSize / 2);

        Addr tag = branchAddr >> (instShiftAmt + 10);
        bool updateTaken = false;
        bool updateNotTaken =false;


        //if the final prediction is from the choice counter
        /*if(history->takenUsed == 0) {
            if(taken) {
                choiceCounters[choiceHistoryIdx].increment();
            }
            else{
                choiceCounters[choiceHistoryIdx].decrement();
            }
        }*/

        //if choice indicates taken and the outcome is not taken, store bias in not taken cache
        if(history->choicePred) {
            if(!taken)
                updateNotTaken = true;
        }
        else {
            if(taken)
                updateTaken = true;
        }


        if(history->takenUsed == 1) {
            updateTaken = true;
        }

        if(history->takenUsed == 2) {
            updateNotTaken = true;
        }

        if(updateTaken)            
            updateTakenCacheValue(tag, LRUIdx, globalHistoryIdx, taken); 
        if(updateNotTaken)
            updateNotTakenCacheValue(tag, LRUIdx, globalHistoryIdx, taken); 
    /*  if (history->takenUsed) {
            // if the taken array's prediction was used, update it
            if (taken) {
                takenCounters[globalHistoryIdx].increment();
            } else {
                takenCounters[globalHistoryIdx].decrement();
            }
        } else {
            // if the not-taken array's prediction was used, update it
            if (taken) {
                notTakenCounters[globalHistoryIdx].increment();
            } else {
                notTakenCounters[globalHistoryIdx].decrement();
            }
        }*/

        if (history->finalPred == taken) {
            /* If the final prediction matches the actual branch's
             * outcome and the choice predictor matches the final
             * outcome, we update the choice predictor, otherwise it
             * is not updated. While the designers of the bi-mode
             * predictor don't explicity say why this is done, one
             * can infer that it is to preserve the choice predictor's
             * bias with respect to the branch being predicted; afterall,
             * the whole point of the bi-mode predictor is to identify the
             * atypical case when a branch deviates from its bias.
             */
            if ((history->finalPred == true && history->takenUsed == 1) || (history->finalPred == false && history->takenUsed == 2)) {
                if (taken) {
                    choiceCounters[choiceHistoryIdx].increment();
                } else {
                    choiceCounters[choiceHistoryIdx].decrement();
                }
            }
        } else {
            // always update the choice predictor on an incorrect prediction
            if (taken) {
                choiceCounters[choiceHistoryIdx].increment();
            } else {
                choiceCounters[choiceHistoryIdx].decrement();
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
YagsBP::retireSquashed(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory*>(bp_history);
    delete history;
}

unsigned
YagsBP::getGHR(ThreadID tid, void *bp_history) const
{
    return static_cast<BPHistory*>(bp_history)->globalHistoryReg;
}

void
YagsBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}


bool
YagsBP::hitInTaken(Addr tag, unsigned idx)
{
    for(int i = 0; i < 2; i++) {
        if(tag == takenTags[idx * 2 + i]) {
            if(i == 0)
                takenLRU[idx] = false;
            else
                takenLRU[idx] = true;
            return true;
        }
    }
    return false;
}

bool
YagsBP::hitInNotTaken(Addr tag, unsigned idx)
{
    for(int i = 0; i < 2; i++) {
        if(tag == notTakenTags[idx * 2 + i]) {
            if(i == 0)
                notTakenLRU[idx] = false;
            else
                notTakenLRU[idx] = true;
            return true;
        }
    }
    return false;
}

void
YagsBP::updateNotTakenCache(Addr tag, unsigned idx, unsigned global_idx)
{
    if(hitInNotTaken(tag, idx)) {
        for(int i = 0; i < 2; i++) {
            if(tag == notTakenTags[idx * 2 + i]) {
                if(i == 0) 
                    notTakenLRU[idx] = false;
                else 
                    notTakenLRU[idx] = true;
                notTakenCounters[global_idx].decrement();                
            }
        }    
    }
    else {
        //line 0 is least recently used
        if(notTakenLRU[idx]) {
            notTakenTags[idx * 2] = tag;
            notTakenLRU[idx] = false;
        }
        else {
            notTakenTags[idx * 2 + 1] = tag;
            notTakenLRU[idx] = true;
        }
        notTakenCounters[global_idx].setBits(globalCtrBits);
        notTakenCounters[global_idx].decrement();
    }
}


void
YagsBP::updateTakenCache(Addr tag, unsigned idx, unsigned global_idx)
{
    if(hitInTaken(tag, idx)) {
        for(int i = 0; i < 2; i++) {
            if(tag == takenTags[idx * 2 + i]) {
                if(i == 0) 
                    takenLRU[idx] = false;
                else 
                    takenLRU[idx] = true;
                takenCounters[global_idx].increment();                
            }
        }    
    }
    else {
        //line 0 is least recently used
        if(takenLRU[idx]) {
            takenTags[idx * 2] = tag;
            takenLRU[idx] = false;
        }
        else {
            takenTags[idx * 2 + 1] = tag;
            takenLRU[idx] = true;
        }
        takenCounters[global_idx].setBits(globalCtrBits);
        takenCounters[global_idx].increment();
    }
}


void
YagsBP::updateTakenCacheValue(Addr tag, unsigned idx, unsigned global_idx, bool taken)
{
    if(hitInTaken(tag, idx)) {
        for(int i = 0; i < 2; i++) {
            if(tag == takenTags[idx * 2 + i]) {
                if(i == 0) 
                    takenLRU[idx] = false;
                else 
                    takenLRU[idx] = true;
                if(taken)
                    takenCounters[global_idx].increment(); 
                if(!taken)
                    takenCounters[global_idx].decrement();
                if(takenCounters[global_idx].read() <= takenThreshold)
                    takenLRU[idx] = !takenLRU[idx];                       
            }
        }    
    }
    else {
        //line 0 is least recently used
        if(takenLRU[idx]) {
            takenTags[idx * 2] = tag;
            takenLRU[idx] = false;
        }
        else {
            takenTags[idx * 2 + 1] = tag;
            takenLRU[idx] = true;
        }
        takenCounters[global_idx].setBits(globalCtrBits);
        if(taken)
            takenCounters[global_idx].increment(); 
        if(!taken)
            takenCounters[global_idx].decrement();
        if(takenCounters[global_idx].read() <= takenThreshold)
            takenLRU[idx] = !takenLRU[idx];
    }
}

void
YagsBP::updateNotTakenCacheValue(Addr tag, unsigned idx, unsigned global_idx, bool taken)
{
    if(hitInNotTaken(tag, idx)) {
        for(int i = 0; i < 2; i++) {
            if(tag == notTakenTags[idx * 2 + i]) {
                if(i == 0) 
                    notTakenLRU[idx] = false;
                else 
                    notTakenLRU[idx] = true;
                if(taken)
                    notTakenCounters[global_idx].increment(); 
                if(!taken)
                    notTakenCounters[global_idx].decrement();
                if(notTakenCounters[global_idx].read() <= notTakenThreshold)
                    notTakenLRU[idx] = !notTakenLRU[idx];                       
            }
        }    
    }
    else {
        //line 0 is least recently used
        if(notTakenLRU[idx]) {
            notTakenTags[idx * 2] = tag;
            notTakenLRU[idx] = false;
        }
        else {
            notTakenTags[idx * 2 + 1] = tag;
            notTakenLRU[idx] = true;
        }
        notTakenCounters[global_idx].setBits(globalCtrBits);
        if(taken)
            notTakenCounters[global_idx].increment(); 
        if(!taken)
            notTakenCounters[global_idx].decrement();
        if(notTakenCounters[global_idx].read() <= notTakenThreshold)
            notTakenLRU[idx] = !notTakenLRU[idx];
    }
}

YagsBP*
YagsBPParams::create()
{
    return new YagsBP(this);
}
