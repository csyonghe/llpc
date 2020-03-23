/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/
/**
 ***********************************************************************************************************************
 * @file  llpcNggLdsManager.h
 * @brief LLPC header file: contains declaration of class lgc::NggLdsManager.
 ***********************************************************************************************************************
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

#include "llpcInternal.h"

namespace lgc
{

class PipelineState;

// Enumerates the types of LDS regions used in NGG.
enum NggLdsRegionType
{
    // LDS region for ES only (no GS)
    LdsRegionDistribPrimId = 0,         // Distributed primitive ID (a special region, overlapped with the region of
                                        //   position data in NGG non pass-through mode)
    LdsRegionPosData,                   // Position data to export
    LdsRegionDrawFlag,                  // Draw flag indicating whether the vertex survives
    LdsRegionPrimCountInWaves,          // Primitive count accumulated per wave (8 potential waves) and per sub-group
    LdsRegionVertCountInWaves,          // Vertex count accumulated per wave (8 potential waves) and per sub-group
    LdsRegionCullDistance,              // Aggregated sign value of cull distance (bitmask)
    // Below regions are for vertex compaction
    LdsRegionVertThreadIdMap,           // Vertex thread ID map (uncompacted -> compacted)
    LdsRegionCompactVertexId,           // Vertex ID (VS only)
    LdsRegionCompactInstanceId,         // Instance ID (VS only)
    LdsRegionCompactPrimId,             // Primitive ID (VS only)
    LdsRegionCompactTessCoordX,         // X of tessCoord (U) (TES only)
    LdsRegionCompactTessCoordY,         // Y of tessCoord (V) (TES only)
    LdsRegionCompactPatchId,            // Patch ID (TES only)
    LdsRegionCompactRelPatchId,         // Relative patch ID (TES only)

    LdsRegionCompactBeginRange = LdsRegionVertThreadIdMap,
    LdsRegionCompactEndRange = LdsRegionCompactRelPatchId,

    LdsRegionEsBeginRange = LdsRegionDistribPrimId,
    LdsRegionEsEndRange = LdsRegionCompactRelPatchId,

    // LDS region for ES-GS
    LdsRegionEsGsRing,                  // ES-GS ring
    LdsRegionOutPrimData,               // GS output primitive data
    LdsRegionOutVertCountInWaves,       // GS output vertex count accumulated per wave (8 potential waves) and per
                                        //   sub-group for each stream (4 GS streams)
    LdsRegionOutVertOffset,             // GS output vertex (exported vertex data) offset in GS-VS ring
                                        //   (overlapped with the region of exported primitive data, LDS reused)
    LdsRegionGsVsRing,                  // GS-VS ring

    LdsRegionGsBeginRange = LdsRegionEsGsRing,
    LdsRegionGsEndRange = LdsRegionGsVsRing,

    // Total
    LdsRegionCount
};

// Size of a DWORD
static const uint32_t SizeOfDword = sizeof(uint32_t);

// =====================================================================================================================
// Represents the manager doing shader merge operations.
class NggLdsManager
{
public:
    NggLdsManager(llvm::Module* pModule, PipelineState* pPipelineState, llvm::IRBuilder<>* pBuilder);

    static uint32_t CalcEsExtraLdsSize(PipelineState* pPipelineState);
    static uint32_t CalcGsExtraLdsSize(PipelineState* pPipelineState);

    // Gets the LDS starting offset for the specified region
    uint32_t GetLdsRegionStart(NggLdsRegionType region) const
    {
        uint32_t regionStart = m_ldsRegionStart[region];
        assert(regionStart != InvalidValue);
        return regionStart;
    }

    llvm::Value* ReadValueFromLds(llvm::Type* pReadTy, llvm::Value* pLdsOffset, bool useDs128 = false);
    void WriteValueToLds(llvm::Value* pWriteValue, llvm::Value* pLdsOffset, bool useDs128 = false);

    void AtomicOpWithLds(llvm::AtomicRMWInst::BinOp atomicOp, llvm::Value* pAtomicValue, llvm::Value* pLdsOffset);

private:
    NggLdsManager() = delete;
    NggLdsManager(const NggLdsManager&) = delete;
    NggLdsManager& operator=(const NggLdsManager&) = delete;

    // -----------------------------------------------------------------------------------------------------------------

    static const uint32_t LdsRegionSizes[LdsRegionCount];  // LDS sizes for all LDS region types (in BYTEs)
    static const char*    LdsRegionNames[LdsRegionCount];  // Name strings for all LDS region types

    PipelineState*  m_pPipelineState; // Pipeline state
    llvm::LLVMContext*        m_pContext;     // LLVM context

    llvm::GlobalValue*  m_pLds;     // Global variable to model NGG LDS

    uint32_t        m_ldsRegionStart[LdsRegionCount]; // Start LDS offsets for all available LDS region types (in BYTEs)

    uint32_t        m_waveCountInSubgroup; // Wave count in sub-group

    llvm::IRBuilder<>*  m_pBuilder; // LLVM IR builder
};

} // lgc
