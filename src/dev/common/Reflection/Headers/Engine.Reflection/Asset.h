#pragma once
#include "Reflection.h"

namespace rw
{
class Asset : public Symbol
{
    DECLARE_ABSTRACT_SYMBOL (Core, Asset, Symbol)
    DECLARE_MEMBER (String, Name)

public:
    virtual ~Asset() = default;
};

class TestBedAsset : public Asset
{
    DECLARE_SYMBOL (Core, TestBedAsset, Asset)
    DECLARE_MEMBER(String, Name2)
};

struct AssetRef
{
    AssetRef()
        : m_Asset(nullptr)
        , m_IDs()
    {}

    DECLARE_STRUCT (Core, AssetRef)
    DECLARE_MEMBER (SmartHandle<Asset>, Asset)
    DECLARE_ARRAY_MEMBER (u32, IDs)
};

DECLARE_ENUM(Core, PlatformEnum, Win32)
}