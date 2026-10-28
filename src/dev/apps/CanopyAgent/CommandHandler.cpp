#include "CommandHandler.h"
#include <Engine.Core/diag/Environment.h>
#include <Engine.Core/diag/ExecutionContext.h>
#include <Engine.ToolCore/App/ApplicationBase.h>
#include <Engine.Storage/Pools/RemoteStoragePool.h>
#include <Engine.Network/Netcrypt.h>

namespace rw
{
static RemoteStoragePool* s_Storage;

bool CanopyAgent_CLI_Init()
{
    if(s_Storage)
        return true;

    // const char* host = Environment::getVariable("CANOPY_DISK_HOST");
    // if(isnil(host))
    // {
    //     APPERRLOG("no host provided from CANOPY_DISK_HOST");
    //     return false;
    // }

    // s_Storage = new RemoteStoragePool(host);
    return true;
}

bool CanopyAgent_CLI_Handler()
{
    // if(!s_Storage)
    // {
    //     APPERRLOG("storage not initialized");
    //     return false;
    // }

    if(const char* OPTV_upload = ExecutionContext::getOptionValue("upload"))
    {
        APPLOGF("Uploading %s...", OPTV_upload);

        SharedBuffer* targetFileBuffer = DEVICE.shareFile(OPTV_upload);
        if(!targetFileBuffer)
        {
            APPERRLOG("File couldn't be read.");
            return false;
        }

        targetFileBuffer->ensureResources();

        InsertResult result = s_Storage->putChunk(targetFileBuffer);
        switch(result)
        {
            default:
                break;

            case InsertResult::Inserted:
                break;

            case InsertResult::AlreadyPresent:
                break;

            case InsertResult::Error:
                APPERRLOG("An error occured while trying to upload to remote disk.");
                return false;
                
            case InsertResult::NoSpace:
                APPERRLOG("No space on remote disk.");
                return false;

            case InsertResult::NullFeedback:
                APPERRLOG("No data completed.");
                return false;

            case InsertResult::NoSourceSpecified:
                APPERRLOG("No source specified on the Canopy Server (code bug).");
                return false;

            case InsertResult::BaseNotFound:
                APPERRLOG("No base data found in the catalog.");
                return false;

            case InsertResult::DeltaNotComputed:
                APPERRLOG("No delta was created due to an error in the computation.");
                return false;
        }

        ContentId contentId;
        contentId.calculate(targetFileBuffer->bytes(), targetFileBuffer->size64());

        FixedStringBuilder<> sContentId;
        tohexstr(&contentId, sizeof(contentId), sContentId);

        ELOGF("File uploaded. TARGET: %s");
        return true;
    }
    
    if(const char* OPTV_fetch = ExecutionContext::getOptionValue("fetch"))
    {
        FetchResult err;

        ContentId contentId;
        fromhexstr(OPTV_fetch, strlen(OPTV_fetch), &contentId, sizeof(contentId));

        SharedBuffer* content;
        err = s_Storage->getChunk(contentId, &content);
        if(err < FetchResult::Found)
        {
            APPERRLOGF("Content couldn't be fetched due to an error (%d) on the Canopy Server. Did you specify the correct Content ID?", err);
            return false;
        }

        if(const char* OPTV_path = ExecutionContext::getOptionValue("path"))
        {
            APPLOGF("Saving %s to file %s...", OPTV_fetch, OPTV_path);
            
            ScopedRef targetFileBuffer = DEVICE.createFile(OPTV_path, NO, NO);
            if(targetFileBuffer.isNull())
            {
                APPERRLOG("File couldn't be created.");
                return false;
            }

            targetFileBuffer->setPosition(0);
            if(targetFileBuffer->write(content->bytes(), content->size64()) < E_BIO_SUCCESS)
            {
                APPERRLOG("File couldn't be saved (written to disk).");
                return false;
            }

            APPLOG("Content fetched.");
        }
        else
        {
            printf("%s", content->bytes());
        }

        return true;
    }

    if(ExecutionContext::getOption("bless"))
    {
        // StringBuilder<> tmp;
        // const char* transactions_path = DEVICE.nativePathTranslation(OPTV_bless, tmp);
        return true;
    }

    // APPLOGF("No option was selected.");
    // APPLOGF("Available: ");
    // APPLOGF("upload (virtual path)");
    // APPLOGF("fetch (content id)");
    return true;
}
}