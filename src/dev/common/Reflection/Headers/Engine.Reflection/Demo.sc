USING "SC_exec.sch" // base

USING "SC_core.sch"
USING "SC_coreRender.sch"

SHARED_BUFFER           superBundleBuffer
DB_OBJECT               superBundle

CONTENT_PARTITION       SP_MainCity
ASSET                   SP_MainCity_Asset

PROC FS_INIT()
    FILE_SYSTEM FS = env.FS()
    FS.Mount("/native_root", "C:\\Games\\Mirrors Edge Catalyst\\")

    superBundleBuffer = FS.Share("/native_root/Data/Win32/anchor.sb")
    _ctor(superBundle, superBundleBuffer.Bytes(), OWNERSHIP_SHARED)

    SP_MainCity = FS.CreatePartition("Configurations/Win32/Level/SP_MainCity", _type(Blueprint))
    SP_MainCity_Asset = SP_MainCity.Asset()
ENDPROC

UI_CONTEXT              UIContext
UI_PANEL                ContentBrowserUI
UI_PANEL                PropertiesUI

PROC UI_SETUP_CONTENT_BROWSER_PANEL(UI_PANEL panel)
    UI_PANEL_INIT(panel, "Content Browser");

    UI_TREENODE objectNode
    UI_TREENODE_INIT(objectNode, "<root>")
    FS_BUILD_UI_TREE(objectNode)

    UI_TREEVIEW treeView
    UI_TREEVIEW_INIT(treeView, objectNode)

    UI_PANEL_ADD_WIDGET(panel, treeView)
ENDPROC

PROC UI_INIT_PROPERTIES_PANEL(UI_PANEL panel)
    UI_PANEL_INIT(panel, "Properties")
ENDPROC

PROC UI_INIT()
    UI_SETUP_CONTENT_BROWSER_PANEL(ContentBrowserUI)
    UI_CONTEXT_ADD_SURFACE(UIContext, ContentBrowserUI)

    UI_INIT_PROPERTIES_PANEL(PropertiesUI)
    UI_CONTEXT_ADD_SURFACE(UIContext, PropertiesUI)
ENDPROC

script Demo
    CORE_RENDER_INIT(1280, 800, env.APP_ID())
    FS_INIT()
    UI_INIT()

    WHILE NOT CORE_RENDER_SHOULD_CLOSE()
        UIContext.PreRenderFrame()
        UIContext.RenderFrame()
        UIContext.PostRenderFrame()
    ENDWHILE

    CORE_RENDER_SHUTDOWN()
endscript