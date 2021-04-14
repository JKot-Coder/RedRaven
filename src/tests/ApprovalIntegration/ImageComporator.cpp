#include "ImageComparator.hpp"

#include "DirectXTex.h"

namespace OpenDemo
{
    namespace Tests
    {
        bool ImageComparator::contentsAreEquivalent(std::string receivedPath, std::string approvedPath) const
        {
            HRESULT result;

            DirectX::TexMetadata receivedMetadata;
            DirectX::ScratchImage receivedImage;
            result = DirectX::LoadFromDDSFile(StringConversions::UTF8ToWString(receivedPath).c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &receivedMetadata, receivedImage);
            ASSERT(SUCCEEDED(result));

            DirectX::TexMetadata approvedMetadata;
            DirectX::ScratchImage approvedImage;
            result = DirectX::LoadFromDDSFile(StringConversions::UTF8ToWString(approvedPath).c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &approvedMetadata, approvedImage);
            ASSERT(SUCCEEDED(result));

            bool equal = (approvedImage.GetPixelsSize() == receivedImage.GetPixelsSize());
            equal &= memcmp(approvedImage.GetPixels(), receivedImage.GetPixels(), approvedImage.GetPixelsSize()) == 0;

            return equal;
        }
    }
}