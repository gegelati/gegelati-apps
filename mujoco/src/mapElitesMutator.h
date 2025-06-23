

#ifndef MAP_ELITES_MUTATOR_H
#define MAP_ELITES_MUTATOR_H

#include <gegelati.h>
#include "mapElitesArchive.h"

namespace Mutator {

    namespace MapElitesMutator {


            void addRandomActionEdge(
                TPG::TPGGraph& graph, const TPG::TPGAction& action,
                const MapElitesArchive& mapElitesArchive,
                const std::vector<uint64_t>& indicesCloned,
                const std::set<std::vector<uint64_t>>& validIndices,
                Mutator::RNG& rng, bool useOnlyCloseAddEdges);

            void mutateTPGAction(
                TPG::TPGGraph& graph, const TPG::TPGAction& action,
                const MapElitesArchive& mapElitesArchive,
                const std::vector<uint64_t>& indicesCloned,
                const std::set<std::vector<uint64_t>>& validIndices,
                std::list<std::shared_ptr<Program::Program>>& newPrograms,
                const Mutator::MutationParameters& params, Mutator::RNG& rng,
                bool useOnlyCloseAddEdges);

            void populateTPG(
                TPG::TPGGraph& graph, const Archive& archive,
                const MapElitesArchive& mapElitesArchive,
                const Mutator::MutationParameters& params, Mutator::RNG& rng,
                uint64_t nbActions,
                uint64_t maxNbThreads = std::thread::hardware_concurrency(),
                bool usePonderationSelection = false,
                bool useOnlyCloseAddEdges = false);
    }
}

#endif