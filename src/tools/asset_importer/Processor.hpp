#pragma once

#include "AssetImporter.hpp"
#include <filesystem>

namespace RR
{
    namespace Common
    {
        enum class RResult : int32_t;
    }

    namespace AssetImporter
    {
        struct Asset
        {
            std::filesystem::path path;
        };

        class ProcessorContext
        {

        };

        struct ProcessorOutput
        {
            enum class Type
            {
                Output,
            };

            std::filesystem::path path;
            Type type;
        };

        class Processor
        {
        public:
            virtual std::vector<U8String> GetListOfExtensions() const = 0;
            //virtual Common::RResult CollectDependencies
            virtual Common::RResult Process(const Asset& asset, const ProcessorContext& context, std::vector<ProcessorOutput>& outputs) const = 0;
        };

        namespace Processors
        {
            template <typename T>
            class ProcessorRegistrator
            {
            public:
                ProcessorRegistrator() { AssetImporter().Instance().Register(processor); };

            private:
                T processor;
            };
        }
    }
}

#define REGISTER_PROCESSOR(processor) \
    static ProcessorRegistrator<processor> processor##ProcessorRegistratorInst;