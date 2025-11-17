#include "ReflectionBuilder.hpp"

#include "effect_library/ShaderReflectionData.hpp"

#include "slang.h"
using namespace slang;

#include <iostream>

#include "ProgramVersion.hpp"
#include "ProgramReflection.hpp"
using namespace Falcor;

#include "common/Result.hpp"

namespace RR
{
    void ReflectionBuilder::Build(slang::ShaderReflection* reflection)
    {
        std::cout << "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" << std::endl;

        std::string log;
        auto programVersion = eastl::make_unique<Falcor::ProgramVersion>(reflection->getSession());
        try {

            std::vector<slang::EntryPointLayout*> entryPointLayouts;

            for(uint32_t i = 0; i < reflection->getEntryPointCount(); i++)
                entryPointLayouts.emplace_back(reflection->getEntryPointByIndex(i));

            auto programReflection = ProgramReflection::create(programVersion.get(), reflection, entryPointLayouts, log);

            auto defaultParameterBlock = programReflection->getDefaultParameterBlock();

           for(uint32_t i = 0; i < defaultParameterBlock->getResourceCount(); i++)
            {
                auto resource = defaultParameterBlock->getResource(i);
                auto resourceBinding = defaultParameterBlock->getResourceBinding(resource->getName());

                std::cout << "resource: " << resource->getName() << " binding location: " << resourceBinding.getResourceRangeIndex() << std::endl;
            }

            for(uint32_t j = 0; j < defaultParameterBlock->getResourceRangeCount(); j++)
            {
                auto castFlavor = [](ParameterBlockReflection::ResourceRangeBindingInfo::Flavor flavor) {
                    switch(flavor)
                    {
                        case ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::Simple: return "Simple";
                        case ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::RootDescriptor: return "RootDescriptor";
                        case ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ConstantBuffer: return "ConstantBuffer";
                        case ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::ParameterBlock: return "ParameterBlock";
                        case ParameterBlockReflection::ResourceRangeBindingInfo::Flavor::Interface: return "Interface";
                    };
                };


                auto bindingInfo = defaultParameterBlock->getResourceRangeBindingInfo(j);
                std::cout << "resource range index: " << j << std::endl;
                std::cout << "flavor: " << castFlavor(bindingInfo.flavor) << std::endl;
                std::cout << "register index: " << bindingInfo.regIndex  << std::endl;
                std::cout << "bind space: " << bindingInfo.regSpace << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "Error creating program reflection: " << e.what() << std::endl;
            //return Common::RResult::Fail;
        }

        std::cout << "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" << std::endl;
    }
}