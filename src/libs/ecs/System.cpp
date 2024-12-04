#include "System.hpp"

#include "EASTL/vector_multimap.h"
#include "EASTL/bitvector.h"
#include "EASTL/sort.h"

namespace RR::Ecs
{
    namespace
    {
        template <class MarkContainer, class ListContainer, class EdgeContainer, typename LoopDetectedCB>
        static bool visitTopSort(uint32_t nodeIdx, const EdgeContainer& edges, MarkContainer& tempMark, MarkContainer& visitedMark,
                                 ListContainer& sortedList, LoopDetectedCB cb)
        {
            if (visitedMark[nodeIdx])
                return true;

            if (tempMark[nodeIdx])
            {
                cb(nodeIdx, tempMark);
                visitedMark.set(nodeIdx, true);
                return false;
            }
            tempMark.set(nodeIdx, true);

            const auto range = edges.equal_range(nodeIdx);
            for (auto edge = range.first; edge != range.second; ++edge)
            {
                if (!visitTopSort(edge->second, edges, tempMark, visitedMark, sortedList, cb))
                    return false;
            }

            tempMark.set(nodeIdx, false);
            sortedList.push_back((uint32_t)nodeIdx);
            visitedMark.set(nodeIdx, true);
            return true;
        }

        // https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
        template <class ListContainer, class EdgeContainer, typename LoopDetectedCB>
        static bool topoSort(uint32_t N, const EdgeContainer& edges, ListContainer& sortedList, LoopDetectedCB cb)
        {
            sortedList.reserve(N);
            eastl::bitvector tempMark(N, false);
            eastl::bitvector visitedMark(N, false);
            for (uint32_t i = 0; i < N; ++i)
            {
                if (!visitTopSort(i, edges, tempMark, visitedMark, sortedList, cb))
                    return false;
            }
            return true;
        }
    }

    void SystemStorage::RegisterDeffered()
    {
        if (!isDirty)
            return;

        Common::Threading::UniqueLock<Common::Threading::Mutex> lock(mutex);

        eastl::vector<SystemDescription*> tmpSystemList;
        for (auto& [key, value] : descriptions)
            tmpSystemList.push_back(&value);

        // Sort by id to avoid depending on native ES registration order
        // which might be different on different platforms, depend on hot-reload, etc...
        eastl::sort(tmpSystemList.begin(), tmpSystemList.end(), [](auto a, auto b) { return a->hashName < b->hashName; });

        // Map hash of name to more simple global index
        eastl::unordered_map<HashSystemType, uint32_t> systemHashToIdxMap;

        for (uint32_t i = 0; i < tmpSystemList.size(); i++)
            systemHashToIdxMap[tmpSystemList[i]->hashName.hash] = i;

        constexpr uint32_t invalidId = std::numeric_limits<uint32_t>::max();

        auto systemHashToIdx = [&systemHashToIdxMap](HashSystemType hash) {
            const auto it = systemHashToIdxMap.find(hash);
            return it != systemHashToIdxMap.end() ? it->second : invalidId;
        };

        eastl::vector_multimap<uint32_t, uint32_t> edges;
        auto makeEdge = [&](uint32_t fromIdx, uint32_t toIdx) { edges.emplace(fromIdx, toIdx); };
        UNUSED(makeEdge);

        auto insertOrderEdge = [&](HashSystemName system, HashSystemName other, bool before) {
            const auto systemIdx = systemHashToIdx(system.hash);
            const auto otherIdx = systemHashToIdx(other.hash);
            ASSERT(systemIdx != invalidId); // Impossible

            if UNLIKELY (otherIdx == invalidId)
            {
                Log::Format::Error("ES <{}> is supposed to be {} ES <{}>, which is undeclared.", system.string.c_str(),
                                   before ? "before" : "after", other.string.c_str());
                return;
            }

            makeEdge(before ? systemIdx : otherIdx, before ? systemIdx : systemIdx);
        };

        // Build edges based on before/after
        for (const auto system : tmpSystemList)
        {
            for (const auto& other : system->before)
                insertOrderEdge(system->hashName, other, false);

            for (const auto& other : system->before)
                insertOrderEdge(system->hashName, other, true);
        }

        eastl::vector<uint32_t> sortedList;
        auto loopDetected = [&](size_t idx, auto&) {
            Log::Format::Error("ES <{}> in graph to become cyclic and was removed from sorting. ES order is non-determinstic.",
                               tmpSystemList[idx]->hashName.string.c_str());
        };
        topoSort((uint32_t)tmpSystemList.size(), edges, sortedList, loopDetected);

        systems.clear();

        for (const auto id : sortedList)
            systems.push_back(tmpSystemList[sortedList[id]]);

        isDirty = false;
    }
}