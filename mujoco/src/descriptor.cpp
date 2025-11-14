
#include "mujocoEnvironment/mujocoWrappers.h"
#include "descriptors.h"

void Selector::MapElites::CustomDescriptors::FeetContact::initDescriptor(
    const TPG::TPGGraph& graph,
    const Learn::LearningEnvironment& learningEnvironment)
{
    this->init = true;
    this->minRange = 0.0;
    this->maxRange = 1.0;

    if(dynamic_cast<const MujocoWrapper*>(&learningEnvironment) == nullptr){
        throw std::runtime_error("FeetContact descriptor can only be used with MujocoWrapper learning environment");
    }
    const auto* castedEnv = dynamic_cast<const MujocoWrapper*>(&learningEnvironment);
    this->nbDescriptors = castedEnv->getFeetGeomIds().size();

    if(this->nbDescriptors == 0){
        throw std::runtime_error("FeetContact descriptor: no foot geom identified in the MujocoWrapper");
    }
}

std::string Selector::MapElites::CustomDescriptors::FeetContact::getName() const
{
    return "FeetContact";
}

void Selector::MapElites::CustomDescriptors::FeetContact::extractMetricsStep(
    std::vector<double>& metrics, const TPG::TPGVertex* agent,
    std::vector<double> actionValues,
    const Learn::LearningEnvironment& learningEnvironment) const
{

    if(dynamic_cast<const MujocoWrapper*>(&learningEnvironment) == nullptr){
        throw std::runtime_error("FeetContact descriptor can only be used with MujocoWrapper learning environment");
    }
    const auto* castedEnv = dynamic_cast<const MujocoWrapper*>(&learningEnvironment);

	std::vector<double> descriptorsValues(castedEnv->getFeetGeomIds().size(), 0.0);
	std::set<uint64_t> increasedLegs;

    for (int i = 0; i < castedEnv->d_->ncon; ++i) {
        const mjContact& contact = castedEnv->d_->contact[i];
        int geom1 = contact.geom1;
        int geom2 = contact.geom2;

        // Check if geom1 is a foot
        auto it1 = castedEnv->getFootGeomToIndex().find(geom1);
        if (it1 != castedEnv->getFootGeomToIndex().end() && increasedLegs.find(it1->second) == increasedLegs.end()) {
            descriptorsValues[it1->second]++;
            increasedLegs.insert(it1->second);
            continue;
        }

        // Check if geom2 is a foot
        auto it2 = castedEnv->getFootGeomToIndex().find(geom2);
        if (it2 != castedEnv->getFootGeomToIndex().end() && increasedLegs.find(it2->second) == increasedLegs.end()) {
            descriptorsValues[it2->second]++;
            increasedLegs.insert(it2->second);
        }
    }

    for(size_t idx = 0; idx < nbDescriptors; idx++) {
        if(descriptorsValues[idx] > 0.0){
            metrics[idx] += 1.0;
        }
    }
}

void Selector::MapElites::CustomDescriptors::FeetContact::
    extractMetricsEpisode(
        std::vector<double>& metrics, const TPG::TPGVertex* agent,
        size_t nbStepsExecuted,
        const Learn::LearningEnvironment& learningEnvironment) const
{
    for (size_t idx = 0; idx < nbDescriptors; idx++) {
        metrics[idx] /= nbStepsExecuted;
    }
}