#pragma once
#include <Engine.ToolCore/App/ApplicationBase.h>

struct WOLFSSL_CTX;

namespace rw
{
class ACanopyAgent : public Application
{
public:
    ACanopyAgent(int argc, const char* argv[]);
    ~ACanopyAgent();

    virtual bool               InitCore() override;
    virtual bool               ShutdownCore() override;

    virtual const char*        getBundleID() const override;
    virtual const char*        getDisplayString() const override;

    int onStart() override;

    void enableBatch();
    bool batchEnabled();

private:
    const char** m_argv;
    int m_argc;
    void* m_tlsContext;

    struct
    {
        u32 batch : 1;
        u32 rest : 31;
    } m_options;
};
}