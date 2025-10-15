
#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <string>

namespace Descriptor{

    enum class DescriptorType {
        FeetContact,
        ActionValues,
        NbInstr,
        NbInstrUseful,
        NbUniqueInstr,
        NbUniqueInstrUseful
    };

    inline std::string descriptorTypeToString(DescriptorType type) {
        switch (type) {
            case DescriptorType::FeetContact: return "FeetContact";
            case DescriptorType::ActionValues: return "ActionValues";
            case DescriptorType::NbInstr: return "NbInstr";
            case DescriptorType::NbInstrUseful: return "NbInstrUseful";
            case DescriptorType::NbUniqueInstr: return "NbUniqueInstr";
            case DescriptorType::NbUniqueInstrUseful: return "NbUniqueInstrUseful";
            default: throw std::runtime_error("Type not known.");
        }
    }

    inline DescriptorType StringToDescriptorType(const std::string& typeStr) {
        if (typeStr == "FeetContact") {
            return DescriptorType::FeetContact;
        } else if (typeStr == "ActionValues") {
            return DescriptorType::ActionValues;
        } else if (typeStr == "NbInstr") {
            return DescriptorType::NbInstr;
        } else if (typeStr == "NbInstrUseful") {
            return DescriptorType::NbInstrUseful;
        } else if (typeStr == "NbUniqueInstr") {
            return DescriptorType::NbUniqueInstr;
        } else if (typeStr == "NbUniqueInstrUseful") {
            return DescriptorType::NbUniqueInstrUseful;
        } else {
            throw std::runtime_error("Type not known.");
        }
    }

    inline bool isBehavioralDescriptor(DescriptorType type){
        switch (type) {
            case DescriptorType::FeetContact: return true;
            case DescriptorType::ActionValues: return true;

            case DescriptorType::NbInstr: return false;
            case DescriptorType::NbInstrUseful: return false;
            case DescriptorType::NbUniqueInstr: return false;
            case DescriptorType::NbUniqueInstrUseful: return false;
            default: throw std::runtime_error("Type not known.");
        }
    }
    
    inline bool isGenotypicDescriptor(DescriptorType type){
        return !isBehavioralDescriptor(type);
    }
};


#endif