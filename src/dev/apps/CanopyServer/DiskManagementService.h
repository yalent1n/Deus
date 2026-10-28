#include <Engine.Network/http/server.h>
#include <Engine.Storage/Pools/FlexibleStoragePool.h>
#include <Engine.Storage/Pools/FileStoragePool.h>

namespace rw
{
typedef StorageFormat DiskType;
typedef ContentId PieceHash;

class DiskManagementService
{
public:
    DiskManagementService(u16 port, DiskType diskType, const char* root);
    ~DiskManagementService();

    void initializeServiceRouters();

public:
    void update() {
        m_Http.update();
    }

    void shutdown() {
        m_Http.shutdown();
    }

    HttpStatus postDISK(const HttpRequest& req, HttpResponse& res);
    HttpStatus opsDISK(const HttpRequest& req, HttpResponse& res);
    HttpStatus fetchDISK(const HttpRequest& req, HttpResponse& res);
    HttpStatus patchDISK(const HttpRequest& req, HttpResponse& res);

private:
    WorkerThreadPool m_Threads;
    HttpServer m_Http;
    ScopedRef<StoragePool> m_Disk;
};
}