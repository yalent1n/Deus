#include <Engine.Storage/Delta.h>
#include <Engine.Base/Memory/VirtualAlloc.h>
#include <Engine.Base/Vec.h>
#include <Engine.Base/Buffer/Adapter.h>
#include <Engine.Base/String/String.h>
#include <cstring>
#include <stdio.h>
#include <map>
#include <Engine.Base/Crypto/Guid.h>
#include <Engine.Base/Misc/FastDelegate.h>
#include <Engine.Base/HashMap.h>

#define TABLE_2D_A(a, w, r, c) a[((r) * (w)) + (c)]
#define LCS(r, c) TABLE_2D_A(v, headSize, r, c)

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

namespace rw
{
struct MatchingBlock
{
    u32 size;
    u32 baseOffset;
    u32 headOffset;

    MatchingBlock(u32 size = 0, u32 baseOffset = 0, u32 headOffset = 0)
        : size(size)
        , baseOffset(baseOffset)
        , headOffset(headOffset)
    {}
};

typedef Vec<MatchingBlock> MatchingBlocks_t;
typedef Vec<MatchingBlock>::iterator MatchingBlockIt_t;

struct RollingBlock
{
    typedef FastDelegate<Guid(const u8*, u32)> StrongChecksumFunc_t;

    const u8* data;
    u32 offset;
    u32 size;
    u32 rollingChecksum;
    StrongChecksumFunc_t strongChecksumFunc;

    Guid strongChecksum() const
    {
        return strongChecksumFunc(data, size);
    }
};

typedef Vec<RollingBlock> RollingBlocks_t;
typedef std::multimap<u32, u32> RollingBlockMap_t;
typedef std::multimap<u32, u32>::iterator RollingBlockIt_t;

u32 _RollingChecksum(const u8* data, u32 size)
{
    u32 a = 0;
	u32 b = 0;
	for (u32 i = 0; i < size; ++i, ++data)
	{
		a += u32(*data);
		b += (size - i) * u32(*data);
	}
	return (a & 0xFFFF) | (b << 16);
}

Guid _StrongChecksum(const u8* data, u32 size)
{
    return getMd5Guid(data, size);
}

struct DeltaSegment
{
    size_t          baseIndex;
    size_t          baseSize;
    size_t          headIndex;
    size_t          headSize;
    u32             baseChecksum;
    u32             headChecksum;
};

struct DeltaGeneratorContext
{
    const u8*               baseData;
    const u8*               headData;
    u32                     baseSize;
    u32                     headSize;
    Adapter*                stream;
};

static void getDeltaOpsFromLcs(const Vec<Dict<s32, u32>>& vv, s32 d, s32 k, u32 baseSize, u32 headSize, u32 headOffset, DeltaOps_t& result)
{
	u32 x = 0, y = 0;
	DeltaOp currentOp(DeltaOp::READ, 0, 0);
	while (d > 0)
	{
		x = vv[d].find(k)->second;
		y = x - k;

		const Dict<s32, u32>& prevV = vv[d - 1];

		u32 xAbove = 0xFFFFFFFF;
		{
			const Dict<s32, u32>::iterator it = prevV.find(k + 1);
			if (it != prevV.end())
			{
				xAbove = it->second;
			}
		}
		u32 yAbove = xAbove - k - 1;

		u32 xBelow = 0xFFFFFFFF;
		{
			const Dict<s32, u32>::iterator it = prevV.find(k - 1);
			if (it != prevV.end())
			{
				xBelow = it->second;
			}
		}
		u32 yBelow = xBelow - k + 1;

		for (; ; )
		{
			if (x == xAbove && y == yAbove + 1)
			{
				if (currentOp.opCode == DeltaOp::INSERT)
				{
					++currentOp.count;
					--currentOp.index;
				}
				else
				{
					result.push_back(currentOp);
					currentOp = DeltaOp(DeltaOp::INSERT, yAbove + headOffset, 1);
				}
				--d;
				++k;
				break;
			}
			else if (x == xBelow + 1 && y == yBelow)
			{
				if (currentOp.opCode == DeltaOp::SKIP)
				{
					++currentOp.count;
				}
				else
				{
					result.push_back(currentOp);
					currentOp = DeltaOp(DeltaOp::SKIP, 0, 1);
				}
				--d;
				--k;
				break;
			}
			if (currentOp.opCode == DeltaOp::READ)
			{
				++currentOp.count;
			}
			else
			{
				result.push_back(currentOp);
				currentOp = DeltaOp(DeltaOp::READ, 0, 1);
			}
			--x;
			--y;
		}
	}
	if (currentOp.count)
	{
		result.push_back(currentOp);
	}
	u32 initialReadCount = vv[0].find(0)->second;
	if (initialReadCount)
	{
		result.push_back(DeltaOp(DeltaOp::READ, 0, initialReadCount));
	}

	// if (x > 0)
	// {
	// 	result.push_back(DeltaOp(DeltaOp::READ, 0, x));
	// }

	// if (y < headSize)
	// {
	// 	result.push_back(DeltaOp(DeltaOp::INSERT, headOffset + y, headSize - y));
	// }

	// if (x < baseSize)
	// {
	// 	result.push_back(DeltaOp(DeltaOp::SKIP, 0, baseSize - x));
	// }
}

static bool computeLcsDelta(const u8* baseData, u32 baseLength, const u8* headData, u32 headLength, u32 headOffset, DeltaOps_t& result)
{
	const s32 maxOps = baseLength + headLength;

	Dict<s32, u32> v;
	Vec<Dict<s32, u32>> vv;

	v[1] = 0;
	for (s32 d = 0; d <= maxOps; ++d)
	{
		for (s32 k = -d; k <= d; k += 2)
		{
			u32 x;
			if ((k == -d || k != d) && (v[k - 1] < v[k + 1]))
			{
				x = v[k + 1];
			}
			else
			{
				x = v[k - 1] + 1;
			}
			u32 y = x - k;
			v[k] = x;
			while (x < baseLength && y < headLength && baseData[x] == headData[y])
			{
				++x;
				++y;
			}
			v[k] = x;
			if (x >= baseLength && y >= headLength)
			{
				vv.push_back(v);
				DeltaOps_t lcsOps;
				getDeltaOpsFromLcs(vv, d, k, baseLength, headLength, headOffset, lcsOps);
				for (auto it = lcsOps.end() - 1, e = lcsOps.begin(); it >= e; --it)
				{
					result.push_back(*it);
				}
				return true;
			}
		}
		vv.push_back(v);
	}
	return false;
}

static void encodeLcsTable(const u8* baseData, u32 baseSize, const u8* headData, u32 headSize, u32* v, DeltaOps_t& ops)
{
    size_t i = baseSize;
    size_t j = headSize;

    while(i > 0 && j > 0) 
    {
        if (baseData[i-1] == headData[j-1]) 
        {
            DeltaOp op(DeltaOp::READ, i, 0);

            while(i>0 && j>0 && baseData[i-1] == headData[j-1]) 
            {
                op.index--;
                op.count++;
                i--;
                j--;
            }
            ops.push_back(op);
        }
        else if (LCS(i-1,j) > LCS(i,j-1)) 
        {
            DeltaOp op(DeltaOp::SKIP, i, 0);

            while(i > 0 && baseData[i-1] != headData[j-1])
            {
                op.index--;
                op.count++;
                i--;
            }

            ops.push_back(op);
        } 
        else 
        {
            if(!ops.empty() && ops.back().opCode == DeltaOp::INSERT) 
            {
                DeltaOp& op = ops.back();
                op.index--;
                op.count++;
            } else 
            {
                ops.push_back(DeltaOp(DeltaOp::INSERT, j-1, 1));
            }
            j--;
        }
    }

    if(j > 0) 
    {
        DeltaOp op(DeltaOp::INSERT, j, j);        
        while(j > 0) 
        {
            j--; 
            op.index--;
        }
        ops.push_back(op);
    }
    if(i > 0) 
    {
        DeltaOp op(DeltaOp::READ, i, i);        
        while(i > 0) 
        {
            i--; 
            op.index--;
        }
        ops.push_back(op);
    }
}

u32 computeLcsTable(const u8* baseData, const u8* headData, u32* v, u32 headSize, u32 m, u32 n)
{
    // for (int j = 1; j <= n; ++j)
    // {
    //     for (int i = 1; i <= m; ++i)
    //     {
    //         if (baseData[i - 1] == headData[j - 1])
    //             LCS(i, j) = LCS(i - 1, j - 1) + 1;
    //         else
    //             LCS(i, j) = max(LCS(i - 1, j), LCS(i, j -1));
    //     }
    // }

    // return LCS(m, n);
    if(m==0||n==0) { LCS(m,n)=0; return 0; }
    if(LCS(m,n)!=u32(-1)) return LCS(m,n);

    u32 x;
    if(baseData[m-1] == headData[n-1]) {
        x = 1 + computeLcsTable(baseData, headData, v, headSize, m - 1, n - 1);
        LCS(m,n)=x;
        return x;
    }
    x = max(computeLcsTable(baseData, headData, v, headSize, m,n-1), computeLcsTable(baseData, headData, v, headSize, m-1,n));
    LCS(m,n)=x;
    return x;
}

bool computeDeltaOps(const u8* baseData, u32 baseSize, const u8* headData, u32 headSize, DeltaOps_t& result)
{
	u32 tableSize = (baseSize + 1) * (headSize + 1);
	
	u32* v = (u32*) HeapAllocGlobal(tableSize * sizeof(u32));
	for(u32 i = 0; i < tableSize; i++)
		v[i] = u32(-1);

	computeLcsTable(baseData, headData, v, headSize, baseSize, headSize);

	DeltaOps_t lcsOps;
	encodeLcsTable(baseData, baseSize, headData, headSize, v, lcsOps);
	for(DeltaOpIt_t i = lcsOps.end() - 1, e = lcsOps.begin(); i >= e; --i)
		result.push_back(*i);

	HeapFreeGlobal(v);
	return true;
    // return computeLcsDelta(baseData, baseSize, headData, headSize, headSize, result);
}

void createBlocks(const u8* baseData, u32 baseOffset, u32 baseSize, u32 numRollingBlocks, u32 windowSize, RollingBlocks_t& baseBlocks)
{
	const u8* end = &baseData[baseOffset + baseSize];
	
	const u8* toRemove = &baseData[baseOffset];
	u32 checksum = _RollingChecksum(toRemove, windowSize);
	u32 a = checksum & 0xFFFF;
	u32 b = checksum >> 16;
	
    // emit the first rolling block
	baseBlocks.push_back(RollingBlock { baseData, baseOffset++, windowSize, checksum, makeDelegate(&_StrongChecksum) });

	const u8* toAdd = toRemove + windowSize;
	while (toAdd < end)
	{
        // remove outgoing byte, add incoming byte
		a = (a - *toRemove + *toAdd) & 0xFFFF;
		b = (b  - (windowSize) * *toRemove + a) & 0xFFFF;
		checksum = a + (b << 16);

        baseBlocks.push_back(RollingBlock { baseData, baseOffset, windowSize, checksum, makeDelegate(&_StrongChecksum) });
		++baseOffset;
		++toRemove;
		++toAdd;
    }
}

inline constexpr u32 c_maxNumOfRollingBlocks = 256;

void createRollingBlocks(const u8* baseData, u32 baseOffset, u32 baseSize, u32 windowSize, RollingBlocks_t& rollingBlocks, RollingBlockMap_t& rollingBlockMap)
{
    u32 numRollingBlocks = min(c_maxNumOfRollingBlocks, baseSize - windowSize + 1);
	rollingBlocks.reserve(numRollingBlocks);
	createBlocks(baseData, baseOffset, baseSize, numRollingBlocks, windowSize, rollingBlocks);
    
	for (u32 i = 0, e = rollingBlocks.size(); i < e; ++i)
	{
        rollingBlockMap.insert(std::make_pair(rollingBlocks[i].rollingChecksum, i));
	}
}

void createMatchingBlocks(const u8* headData, u32 headOffset, u32 headSize, u32 windowSize, RollingBlocks_t& rollingBlocks, RollingBlockMap_t& rollingBlockMap, MatchingBlocks_t& matchingBlocks)
{
    const u8* toRemove = &headData[headOffset];
	const u8* toAdd = toRemove + windowSize;
	const u8* end = toRemove + headSize;
	
    u32 checksum = _RollingChecksum(toRemove, windowSize);
	u32 a = checksum & 0xFFFF;
	u32 b = checksum >> 16;
    
	u32 headBlockIndex = 0;
	while (toAdd < end)
	{
		bool foundMatch = false;
        auto itp = rollingBlockMap.equal_range(checksum);

        // if a match is found, push it
        if(itp.first != rollingBlockMap.end())
        {
            for(auto it = itp.first; it != itp.second; ++it)
            {
                const RollingBlock& baseBlock = rollingBlocks[it->second];
                if (baseBlock.strongChecksum() == _StrongChecksum(toRemove, windowSize))
                {
                    matchingBlocks.push_back(MatchingBlock(windowSize, baseBlock.offset, headOffset + headBlockIndex));
                    foundMatch = true;
                }
            }
        }
				
        // depending on match result, do different advancing
		if (foundMatch)
		{
			headBlockIndex += windowSize;
			if (headBlockIndex + windowSize > headSize)
			{
				break;
			}
			toRemove = &headData[headOffset + headBlockIndex];
			checksum = _RollingChecksum(toRemove, windowSize);
			toAdd = toRemove + windowSize;
			a = checksum & 0xFFFF;
			b = checksum >> 16;
		}
		else
		{
			++headBlockIndex;
			a = (a - *toRemove + *toAdd) & 0xFFFF;
			b = (b  - (windowSize) * *toRemove + a) & 0xFFFF;
			checksum = a + (b << 16);
			++toRemove;
			++toAdd;
		}
	}
}

bool validateSizes(MatchingBlockIt_t prevMatch, MatchingBlockIt_t nextMatch, u32 windowSize, u32& baseOffset, u32& baseSize, u32& headOffset, u32& headSize)
{
    baseOffset = prevMatch->baseOffset + prevMatch->size;
	baseSize = nextMatch->baseOffset - baseOffset;
	if (baseSize < windowSize)
	{
		return false;
	}
	
	headOffset = prevMatch->headOffset + prevMatch->size;
	headSize = nextMatch->headOffset - headOffset;
	if (headSize < windowSize)
	{
		return false;
	}

    return true;
}

u32 insertMatchingBlocks(
    const u8* baseData, 
    const u8* headData, 
    u32 windowSize, 
    MatchingBlocks_t& result, 
    MatchingBlockIt_t prevMatch,
    MatchingBlockIt_t nextMatch)
{
    u32 baseOffset, baseSize, headOffset, headSize;
    if(!validateSizes(prevMatch, nextMatch, windowSize, baseOffset, baseSize, headOffset, headSize))
    {
        return 0;
    }

    RollingBlocks_t rollingBlocks;
    RollingBlockMap_t rollingBlockMap;
    createRollingBlocks(baseData, baseOffset, baseSize, windowSize, rollingBlocks, rollingBlockMap);
    
	MatchingBlocks_t matchingBlocks;
    createMatchingBlocks(headData, headOffset, headSize, windowSize, rollingBlocks, rollingBlockMap, matchingBlocks);

	u32 bytesMatched = 0;
	if (matchingBlocks.size())
	{
		// Could be changed to ensure we always take the largest possible number of source blocks, for now we are satisfied with greedily taking blocks in order
		struct BlockMatchComparer
		{
			bool operator()(const MatchingBlock& lhs, const MatchingBlock& rhs) const
			{
				if (lhs.baseOffset == rhs.baseOffset)
				{
					return lhs.headOffset < rhs.headOffset;
				}
				return lhs.baseOffset < rhs.baseOffset;
			}
		};
		algorithm::insertion_sort(matchingBlocks.begin(), matchingBlocks.end(), BlockMatchComparer());

		result.insert(nextMatch, matchingBlocks[0]);
		bytesMatched += windowSize;
		MatchingBlock& prev = matchingBlocks[0];
		for (int i = 1, e = matchingBlocks.size(); i < e; ++i)
		{
			const MatchingBlock& current = matchingBlocks[i];
			if (current.baseOffset >= prev.baseOffset + windowSize && current.headOffset >= prev.headOffset + windowSize)
			{
				result.insert(nextMatch, current);
				prev = current;
				bytesMatched += windowSize;
			}
		}
	}

	return bytesMatched;
}

inline constexpr u32 c_minSlidingWindowTreshold = 400;

bool computeLargeDeltaOps(const u8* baseData, u32 baseSize, const u8* headData, u32 headSize, DeltaOps_t& result)
{
    MatchingBlocks_t matchingBlocks;
    matchingBlocks.push_back(MatchingBlock(0, 0, 0));
    matchingBlocks.push_back(MatchingBlock(0, baseSize, headSize));

    u32 windowSize = 512 * 1024;

    while(windowSize >= DeltaGenerator::c_minSlidingWindowSize)
    {
        if (windowSize <= baseSize && windowSize <= headSize)
		{
			MatchingBlockIt_t nextMatch = matchingBlocks.begin();
			MatchingBlockIt_t prevMatch = nextMatch++;
			MatchingBlockIt_t end = matchingBlocks.end();
			while (nextMatch != end)
			{
				insertMatchingBlocks(baseData, headData, windowSize, matchingBlocks, prevMatch, nextMatch);
				prevMatch = nextMatch++;
				struct BlockMatchComparer
				{
					bool operator()(const MatchingBlock& lhs, const MatchingBlock& rhs) const
					{
						if (lhs.baseOffset == rhs.baseOffset)
						{
							return lhs.headOffset < rhs.headOffset;
						}
						return lhs.baseOffset < rhs.baseOffset;
					}
				};
				algorithm::insertion_sort(matchingBlocks.begin(), matchingBlocks.end(), BlockMatchComparer());
			}
            //break;
		}
		windowSize >>= 1;
    }
    
    struct LcsJob
    {
        u32 baseIndex;
        u32 baseSize;
        u32 headIndex;
        u32 headSize;
        u32 readCountAfter;

        DeltaOps_t ops;

        void run(const u8* baseData, const u8* headData)
		{
			if (baseSize && headSize)
			{
				if (!computeLcsDelta(baseData + baseIndex, baseSize, headData + headIndex, headSize, headIndex, ops))
				{
					// LCS delta not found, discard from base and insert from head
					ops.push_back(DeltaOp(DeltaOp::SKIP, 0, baseSize));
					ops.push_back(DeltaOp(DeltaOp::INSERT, headIndex, headSize));
				}
			}
			else if (baseSize)
			{
				// Discard bytes from base
				ops.push_back(DeltaOp(DeltaOp::SKIP, 0, baseSize));
			}
			else if (headSize)
			{
				// Insert bytes from head
				ops.push_back(DeltaOp(DeltaOp::INSERT, headIndex, headSize));
			}
			if (readCountAfter)
			{
				ops.push_back(DeltaOp(DeltaOp::READ, 0, readCountAfter));
			}
		}
    };
    Vec<LcsJob> lcsJobs;

    u32 baseIndex = 0;
	u32 headIndex = 0;
	MatchingBlockIt_t matchingBlock = matchingBlocks.begin();
	//++matchingBlock;
	MatchingBlockIt_t end = matchingBlocks.end();
	while (matchingBlock != end)
	{
		__builtin_assert(matchingBlock->baseOffset >= baseIndex);
		__builtin_assert(matchingBlock->headOffset >= headIndex);
		
		LcsJob& lcsJob = lcsJobs.push_back();
		lcsJob.baseIndex = baseIndex;
		lcsJob.baseSize = matchingBlock->baseOffset - baseIndex;
		lcsJob.headIndex = headIndex;
		lcsJob.headSize = matchingBlock->headOffset - headIndex;
		
		headIndex += lcsJob.headSize;
		baseIndex += lcsJob.baseSize;

		lcsJob.readCountAfter = matchingBlock->size;
		headIndex += matchingBlock->size;
		baseIndex += matchingBlock->size;
		++matchingBlock;
	}

    DeltaOps_t internalOps;
    for(LcsJob& job : lcsJobs)
    {
		job.run(baseData, headData);
        for(DeltaOpIt_t opIt = job.ops.begin(); opIt < job.ops.end(); ++opIt)
        {
            internalOps.push_back(*opIt);
        }
    }

    DeltaOp currentOp(DeltaOp::READ, 0, 0);
	for (const DeltaOp& deltaOp : internalOps)
	{
		if (deltaOp.opCode == currentOp.opCode)
		{
			currentOp.count += deltaOp.count;
		}
		else
		{
			if (currentOp.count)
			{
				result.push_back(currentOp);
			}
			currentOp = deltaOp;
		}
	}
	if (currentOp.count)
	{
		result.push_back(currentOp);
	}

	return true;
}

bool DeltaGenerator::computeDelta(const void* baseData, u32 baseSize, const void* headData, u32 headSize, DeltaOps_t& result)
{
    if(baseSize <= c_minSlidingWindowTreshold && headSize <= c_minSlidingWindowTreshold)
    {
        return computeDeltaOps((const u8*) baseData, baseSize, (const u8*) headData, headSize, result);
    }
    
    return computeLargeDeltaOps((const u8*) baseData, baseSize, (const u8*) headData, headSize, result);
}

void FluxPatchEncoder::convertToReplaceOps(const DeltaOps_t& delta, DeltaReplaceOps_t& result)
{
    u32 baseIndex = 0;
	u32 headIndex = 0;
	u32 deltaOpIndex = 0;
	u32 deltaOpCount = delta.size();
	while (deltaOpIndex < deltaOpCount)
	{
		const DeltaOp& current = delta[deltaOpIndex];
		if (current.opCode == DeltaOp::READ)
		{
			baseIndex += current.count;
			headIndex += current.count;
		}
		else 
		{
			const DeltaOp* next = deltaOpIndex < deltaOpCount - 1 ? &delta[deltaOpIndex + 1] : nullptr;
			if (current.opCode == DeltaOp::SKIP)
			{
				if (next && next->opCode == DeltaOp::INSERT)
				{
					result.push_back(DeltaReplaceOp(baseIndex, current.count, headIndex, next->count));
					headIndex += next->count;
					++deltaOpIndex;
				}
				else
				{
					result.push_back(DeltaReplaceOp(baseIndex, current.count, headIndex, 0));
				}
				baseIndex += current.count;
			}
			else // current.opCode == DeltaGenerator::INSERT
			{
				if (next && next->opCode == DeltaOp::SKIP)
				{
					result.push_back(DeltaReplaceOp(baseIndex, next->count, headIndex, current.count));
					++deltaOpIndex;
					baseIndex += next->count;
				}
				else
				{
					result.push_back(DeltaReplaceOp(baseIndex, 0, headIndex, current.count));
				}
				headIndex += current.count;
			}
		}
		++deltaOpIndex;
	}
}

void FluxPatchEncoder::encodeUncompressedDelta(const u8* headData, const DeltaReplaceOps_t& replaceOps, Buffer& result)
{
	//result.write(u8(FluxPatchType_UncompressedDelta));

	for(const DeltaReplaceOp& replaceOp : replaceOps)
	{
		result.encodeUint(replaceOp.baseOffset);
		result.encodeUint(replaceOp.baseSize);
		result.encodeUint(replaceOp.headSize);
		if(replaceOp.headSize)
			result.write((const u8*) headData + replaceOp.headOffset, replaceOp.headSize);
	}
}

void FluxPatchEncoder::decodeUncompressedDelta(const u8* baseData, u32& baseIndex, Adapter& patchData, Buffer& result)
{
	u32 baseReplaceOffset = patchData.decodeUint();
	u32 baseReplaceSize = patchData.decodeUint();
	u32 headReplaceSize = patchData.decodeUint();

	u32 numOfBaseBytes = baseReplaceOffset - baseIndex;
	result.write((const u8*)baseData + baseIndex, numOfBaseBytes);

	if(headReplaceSize)
		patchData.copyTo(result, headReplaceSize);

	baseIndex = baseReplaceOffset + baseReplaceSize;
}

void FluxPatchEncoder::encodeCompressedDelta(const u8* headData, const DeltaReplaceOps_t& replaceOps, Buffer& result)
{
	result.write(u8(FluxPatchType_CompressedDelta));

	for(const DeltaReplaceOp& replaceOp : replaceOps)
	{
		result.encodeUint(replaceOp.baseOffset);
		result.encodeUint(replaceOp.baseSize);
		result.encodeUint(replaceOp.headSize);
		if(replaceOp.headSize)
		{
			Vec<u8> compressedBuffer;
			u32 encodedSize = m_compressionHelper.compressOneSegment((const u8*)headData + replaceOp.headOffset, replaceOp.headSize, compressedBuffer);
			result.write(compressedBuffer.begin(), encodedSize);
		}
		else
		{
			result.write(u64(0));
		}
	}
}

void FluxPatchEncoder::decodeCompressedDelta(const u8* baseData, u32& baseIndex, Adapter& patchData, Buffer& result)
{
	// u32 baseReplaceOffset = patchData.decodeUint();
	// u32 baseReplaceSize = patchData.decodeUint();

	// Codec::Hdr hdr;
	// patchData.read(&hdr, sizeof(Codec::Hdr));

	// Vec<u8> decompressedBuffer;


	// m_compressionHelper.decompressOneSegment(patchData, )
	// u32 headReplaceSize = patchData.decodeUint();

	// u32 numOfBaseBytes = baseReplaceOffset - baseIndex;
	// result.write((const u8*)baseData + baseIndex, numOfBaseBytes);

	// if(headReplaceSize)
	// 	patchData.copyTo(result, headReplaceSize);

	// baseIndex = baseReplaceOffset + baseReplaceSize;
}

u32 FluxPatchEncoder::encodeDelta(const void* baseData, u32 baseSize, const void* headData, u32 headSize, const DeltaOps_t& delta, Buffer& result)
{
	DeltaReplaceOps_t replaceOps;
	convertToReplaceOps(delta, replaceOps);

	s64 beginPos = result.getPosition();
	encodeUncompressedDelta((const u8*)headData, replaceOps, result);
	s64 endPos = result.getPosition();

	return u32(endPos - beginPos);
}

u32 FluxPatchEncoder::decodeDelta(const void* baseData, u32 baseSize, const void* patchData, u32 patchSize, Buffer& result)
{
	Adapter patchDataBuffer(patchData, patchSize);

	u32 baseIndex = 0;
	while(patchDataBuffer.getPosition() < patchDataBuffer.getAvailableBytes())
	{
		u32 baseReplaceOffset = patchDataBuffer.decodeUint();
		u32 baseReplaceSize = patchDataBuffer.decodeUint();
		u32 headReplaceSize = patchDataBuffer.decodeUint();

		u32 numOfBaseBytes = baseReplaceOffset - baseIndex;
		result.write((const u8*)baseData + baseIndex, numOfBaseBytes);

		if(headReplaceSize)
			patchDataBuffer.copyTo(result, headReplaceSize);

		baseIndex = baseReplaceOffset + baseReplaceSize;
	}

	// add remaining bytes because we don't emit leftovers
	u32 numOfRemainingBytes = baseSize - baseIndex;
	result.write((const u8*)baseData + baseIndex, numOfRemainingBytes);
	return true;
}
}