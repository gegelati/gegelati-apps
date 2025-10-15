

#ifndef MAP_ELITES_MUTATOR_H
#define MAP_ELITES_MUTATOR_H

#include <gegelati.h>
#include "mapElitesArchive.h"

namespace Mutator {

    namespace MapElitesMutator {


            void addRandomActionEdge(
                TPG::TPGGraph& graph, const TPG::TPGAction& action,
                const std::vector<const TPG::TPGAction*>& preExistingActions,
                Mutator::RNG& rng);

            void mutateTPGAction(
                TPG::TPGGraph& graph, const TPG::TPGAction& action,
                const std::vector<const TPG::TPGAction*>& preExistingActions,
                std::list<std::shared_ptr<Program::Program>>& newPrograms,
                const Mutator::MutationParameters& params, Mutator::RNG& rng);

            std::vector<const TPG::TPGAction*> getPreExistingActions(const std::map<Descriptor::DescriptorType, MapElitesArchive*>& mapElitesArchives);

            void populateTPG(
                TPG::TPGGraph& graph, const Archive& archive,
                const std::map<Descriptor::DescriptorType, MapElitesArchive*>& mapEliteArchives,
                const Mutator::MutationParameters& params, Mutator::RNG& rng,
                uint64_t nbActions,
                uint64_t maxNbThreads = std::thread::hardware_concurrency());
    }
}

#endif