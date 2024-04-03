#pragma once

namespace RR
{
    class AssetServerImpl;

    class AssetServer final
    {
    public:
        AssetServer();
        ~AssetServer();
        void Run();

    private:
        std::unique_ptr<AssetServerImpl> impl;
    };
}