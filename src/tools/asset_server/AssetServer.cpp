#include "AssetServer.hpp"
#include "efsw\efsw.hpp"

namespace RR
{
    class AssetServerImpl final : public efsw::FileWatchListener
    {
    public:

        AssetServerImpl() { fileWatcher = std::make_unique<efsw::FileWatcher>();
        }

        void Run()
        {
            Log::Format::Info("Start asset server...\n");

            fileWatcher->addWatch(".", this, false);

            fileWatcher->watch();
        }

        void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                                               efsw::Action action, std::string oldFilename) override;

    private:
        std::unique_ptr<efsw::FileWatcher> fileWatcher;
    };

    void AssetServerImpl::handleFileAction(efsw::WatchID, const std::string& dir, const std::string& filename,
                                           efsw::Action action, std::string oldFilename)
    {

        switch (action)
        {
            case efsw::Actions::Add: Log::Format::Info("DIR (\"{}\") FILE (\"{}\") has event Added \n", dir, filename); break;
            case efsw::Actions::Delete: Log::Format::Info("DIR (\"{}\") FILE (\"{}\") has event Delete \n", dir, filename); break;
            case efsw::Actions::Modified: Log::Format::Info("DIR (\"{}\") FILE (\"{}\")has event Modified  \n", dir, filename); break;
            case efsw::Actions::Moved:
                Log::Format::Info("DIR (\"{}\") FILE (\"{}\") has event Moved from ({}) \n", dir, filename, oldFilename);
                break;
            default: ASSERT(false);
        }
    }

    AssetServer::AssetServer() { impl = std::make_unique<AssetServerImpl>(); }
    AssetServer::~AssetServer() { }

    void AssetServer::Run()
    {
        ASSERT(impl);
        impl->Run();
    }
}