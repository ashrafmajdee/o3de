/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Core/EditorActionsHandler.h>

#include <AzToolsFramework/ActionManager/Action/ActionManagerInterface.h>
#include <AzToolsFramework/ActionManager/Menu/MenuManagerInterface.h>
#include <AzToolsFramework/ActionManager/ToolBar/ToolBarManagerInterface.h>

#include <AzQtComponents/Components/SearchLineEdit.h>

#include <CryEdit.h>
#include <QtViewPaneManager.h>

#include <QDesktopServices>
#include <QMainWindow>
#include <QMenu>
#include <QUrl>
#include <QUrlQuery>
#include <QWidget>

static constexpr AZStd::string_view EditorMainWindowActionContextIdentifier = "o3de.context.editor.mainwindow";

static constexpr AZStd::string_view EditorMainWindowMenuBarIdentifier = "o3de.menubar.editor.mainwindow";

static constexpr AZStd::string_view FileMenuIdentifier = "o3de.menu.editor.file";
static constexpr AZStd::string_view EditMenuIdentifier = "o3de.menu.editor.edit";
static constexpr AZStd::string_view GameMenuIdentifier = "o3de.menu.editor.game";
static constexpr AZStd::string_view ToolsMenuIdentifier = "o3de.menu.editor.tools";
static constexpr AZStd::string_view ViewMenuIdentifier = "o3de.menu.editor.view";
static constexpr AZStd::string_view HelpMenuIdentifier = "o3de.menu.editor.help";
static constexpr AZStd::string_view HelpDocumentationMenuIdentifier = "o3de.menu.editor.help.documentation";
static constexpr AZStd::string_view HelpGameDevResourcesMenuIdentifier = "o3de.menu.editor.help.gamedevresources";

static constexpr AZStd::string_view ToolsToolBarIdentifier = "o3de.toolbar.editor.tools";
static constexpr AZStd::string_view PlayControlsToolBarIdentifier = "o3de.toolbar.editor.playcontrols";

void EditorActionsHandler::Initialize(QMainWindow* mainWindow)
{
    m_mainWindow = mainWindow;
    m_cryEditApp = CCryEditApp::instance();
    m_qtViewPaneManager = QtViewPaneManager::instance();

    m_actionManagerInterface = AZ::Interface<AzToolsFramework::ActionManagerInterface>::Get();
    AZ_Assert(m_actionManagerInterface, "EditorActionsHandler - could not get ActionManagerInterface on EditorActionsHandler construction.");
        
    m_menuManagerInterface = AZ::Interface<AzToolsFramework::MenuManagerInterface>::Get();
    AZ_Assert(m_menuManagerInterface, "EditorActionsHandler - could not get MenuManagerInterface on EditorActionsHandler construction.");
        
    m_toolBarManagerInterface = AZ::Interface<AzToolsFramework::ToolBarManagerInterface>::Get();
    AZ_Assert(m_toolBarManagerInterface, "EditorActionsHandler - could not get ToolBarManagerInterface on EditorActionsHandler construction.");

    InitializeActionContext();
    InitializeActions();
    InitializeMenus();
    InitializeToolBars();

    // Ensure the tools menu and toolbar are refreshed when the viewpanes change.
    QObject::connect(
        m_qtViewPaneManager, &QtViewPaneManager::registeredPanesChanged, m_mainWindow,
        [&]()
        {
            RefreshToolActions();
        }
    );

    AzToolsFramework::EditorEventsBus::Handler::BusConnect();
    m_initialized = true;
}

EditorActionsHandler::~EditorActionsHandler()
{
    if (m_initialized)
    {
        AzToolsFramework::EditorEventsBus::Handler::BusDisconnect();
    }
}

void EditorActionsHandler::InitializeActionContext()
{
    AzToolsFramework::ActionContextProperties contextProperties;
    contextProperties.m_name = "O3DE Editor";

    m_actionManagerInterface->RegisterActionContext("", EditorMainWindowActionContextIdentifier, contextProperties, m_mainWindow);
}

void EditorActionsHandler::InitializeActions()
{
    // --- Level Actions

    // New Level
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "New Level";
        actionProperties.m_description = "Create a new level";
        actionProperties.m_category = "Level";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.level.new", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnCreateLevel();
            }
        );
    }

    // Open Level
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Open Level...";
        actionProperties.m_description = "Open an existing level";
        actionProperties.m_category = "Level";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.level.open", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnOpenLevel();
            }
        );
    }

    // Save
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Save";
        actionProperties.m_description = "Save the current level";
        actionProperties.m_category = "Level";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.level.save", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnFileSave();
            }
        );
    }

    // --- Help Actions

    // Tutorials
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Tutorials";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.tutorials", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationTutorials();
            }
        );
    }

    // Open 3D Engine Documentation
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Open 3D Engine Documentation";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.documentation.o3de", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationO3DE();
            }
        );
    }

    // GameLift Documentation
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "GameLift Documentation";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.documentation.gamelift", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationGamelift();
            }
        );
    }

    // Release Notes
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Release Notes";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.documentation.releasenotes", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationReleaseNotes();
            }
        );
    }

    // GameDev Blog
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "GameDev Blog";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.resources.gamedevblog", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationGameDevBlog();
            }
        );
    }

    // Forums
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "Forums";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.resources.forums", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationForums();
            }
        );
    }

    // AWS Support
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "AWS Support";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.resources.awssupport", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnDocumentationAWSSupport();
            }
        );
    }

    // About O3DE
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "&About O3DE";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.abouto3de", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnAppAbout();
            }
        );
    }

    // Welcome
    {
        AzToolsFramework::ActionProperties actionProperties;
        actionProperties.m_name = "&Welcome";
        actionProperties.m_category = "Help";

        m_actionManagerInterface->RegisterAction(
            EditorMainWindowActionContextIdentifier, "o3de.action.help.welcome", actionProperties,
            [cryEdit = m_cryEditApp]()
            {
                cryEdit->OnAppShowWelcomeScreen();
            }
        );
    }

}

void EditorActionsHandler::InitializeMenus()
{
    // Register MenuBar
    m_menuManagerInterface->RegisterMenuBar(EditorMainWindowMenuBarIdentifier);

    // Initialize Menus
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&File";
        m_menuManagerInterface->RegisterMenu(FileMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&Edit";
        m_menuManagerInterface->RegisterMenu(EditMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&Game";
        m_menuManagerInterface->RegisterMenu(GameMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&Tools";
        m_menuManagerInterface->RegisterMenu(ToolsMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&View";
        m_menuManagerInterface->RegisterMenu(ViewMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "&Help";
        m_menuManagerInterface->RegisterMenu(HelpMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "Documentation";
        m_menuManagerInterface->RegisterMenu(HelpDocumentationMenuIdentifier, menuProperties);
    }
    {
        AzToolsFramework::MenuProperties menuProperties;
        menuProperties.m_name = "GameDev Resources";
        m_menuManagerInterface->RegisterMenu(HelpGameDevResourcesMenuIdentifier, menuProperties);
    }

    // Add Menus to MenuBar
    // We space the sortkeys by 100 to allow external systems to add menus in-between.
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, FileMenuIdentifier, 100);
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, EditMenuIdentifier, 200);
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, GameMenuIdentifier, 300);
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, ToolsMenuIdentifier, 400);
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, ViewMenuIdentifier, 500);
    m_menuManagerInterface->AddMenuToMenuBar(EditorMainWindowMenuBarIdentifier, HelpMenuIdentifier, 600);

    // Set the menu bar for this window
    m_mainWindow->setMenuBar(m_menuManagerInterface->GetMenuBar(EditorMainWindowMenuBarIdentifier));

    // Add actions to each menu

    // File
    {
        m_menuManagerInterface->AddActionToMenu(FileMenuIdentifier, "o3de.action.level.new", 100);
        m_menuManagerInterface->AddActionToMenu(FileMenuIdentifier, "o3de.action.level.open", 200);
        m_menuManagerInterface->AddActionToMenu(FileMenuIdentifier, "o3de.action.level.save", 300);
    }

    // Help - Search Documentation Widget
    {
        m_menuManagerInterface->AddWidgetToMenu(HelpMenuIdentifier, CreateDocsSearchWidget(), 100);
    }

    // Help
    {
        m_menuManagerInterface->AddActionToMenu(HelpMenuIdentifier, "o3de.action.help.tutorials", 200);
        m_menuManagerInterface->AddSubMenuToMenu(HelpMenuIdentifier, HelpDocumentationMenuIdentifier, 300);
        {
            m_menuManagerInterface->AddActionToMenu(HelpDocumentationMenuIdentifier, "o3de.action.help.documentation.o3de", 100);
            m_menuManagerInterface->AddActionToMenu(HelpDocumentationMenuIdentifier, "o3de.action.help.documentation.gamelift", 200);
            m_menuManagerInterface->AddActionToMenu(HelpDocumentationMenuIdentifier, "o3de.action.help.documentation.releasenotes", 300);
        }
        m_menuManagerInterface->AddSubMenuToMenu(HelpMenuIdentifier, HelpGameDevResourcesMenuIdentifier, 400);
        {
            m_menuManagerInterface->AddActionToMenu(HelpGameDevResourcesMenuIdentifier, "o3de.action.help.resources.gamedevblog", 100);
            m_menuManagerInterface->AddActionToMenu(HelpGameDevResourcesMenuIdentifier, "o3de.action.help.resources.forums", 200);
            m_menuManagerInterface->AddActionToMenu(HelpGameDevResourcesMenuIdentifier, "o3de.action.help.resources.awssupport", 300);
        }
        m_menuManagerInterface->AddSeparatorToMenu(HelpMenuIdentifier, 500);
        m_menuManagerInterface->AddActionToMenu(HelpMenuIdentifier, "o3de.action.help.abouto3de", 600);
        m_menuManagerInterface->AddActionToMenu(HelpMenuIdentifier, "o3de.action.help.welcome", 700);
    }
}

void EditorActionsHandler::InitializeToolBars()
{
    // Initialize ToolBars
    {
        AzToolsFramework::ToolBarProperties toolBarProperties;
        toolBarProperties.m_name = "Tools";
        m_toolBarManagerInterface->RegisterToolBar(ToolsToolBarIdentifier, toolBarProperties);
    }

    {
        AzToolsFramework::ToolBarProperties toolBarProperties;
        toolBarProperties.m_name = "Play Controls";
        m_toolBarManagerInterface->RegisterToolBar(PlayControlsToolBarIdentifier, toolBarProperties);
    }

    // Set the toolbars
    m_mainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolBarManagerInterface->GetToolBar(ToolsToolBarIdentifier));
    m_mainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolBarManagerInterface->GetToolBar(PlayControlsToolBarIdentifier));
}

QWidget* EditorActionsHandler::CreateDocsSearchWidget()
{
    QWidget* containerWidget = new QWidget(m_mainWindow);
    auto lineEdit = new AzQtComponents::SearchLineEdit(m_mainWindow);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(lineEdit);
    containerWidget->setLayout(layout);
    containerWidget->setContentsMargins(2, 0, 2, 0);
    lineEdit->setPlaceholderText(QObject::tr("Search documentation..."));

    auto searchAction = [lineEdit]()
    {
        auto text = lineEdit->text();
        if (text.isEmpty())
        {
            QDesktopServices::openUrl(QUrl("https://www.o3de.org/docs/"));
        }
        else
        {
            QUrl docSearchUrl("https://www.o3de.org/search/");
            QUrlQuery docSearchQuery;
            docSearchQuery.addQueryItem("query", text);
            docSearchUrl.setQuery(docSearchQuery);
            QDesktopServices::openUrl(docSearchUrl);
        }
        lineEdit->clear();
    };
    QObject::connect(lineEdit, &QLineEdit::returnPressed, m_mainWindow, searchAction);

    QMenu* helpMenu = m_menuManagerInterface->GetMenu(HelpMenuIdentifier);

    QObject::connect(helpMenu, &QMenu::aboutToHide, lineEdit, &QLineEdit::clear);
    QObject::connect(helpMenu, &QMenu::aboutToShow, lineEdit, &QLineEdit::clearFocus);

    return containerWidget;
}

void EditorActionsHandler::OnViewPaneOpened(const char* viewPaneName)
{
    AZStd::string toolActionIdentifier = AZStd::string::format("o3de.action.tool.%s", viewPaneName);
    m_actionManagerInterface->UpdateAction(toolActionIdentifier);
}

void EditorActionsHandler::OnViewPaneClosed(const char* viewPaneName)
{
    AZStd::string toolActionIdentifier = AZStd::string::format("o3de.action.tool.%s", viewPaneName);
    m_actionManagerInterface->UpdateAction(toolActionIdentifier);
}

void EditorActionsHandler::RefreshToolActions()
{
    // If the tools are being displayed in the menu or toolbar already, remove them.
    m_menuManagerInterface->RemoveActionsFromMenu(ToolsMenuIdentifier, m_toolActionIdentifiers);
    m_toolBarManagerInterface->RemoveActionsFromToolBar(ToolsToolBarIdentifier, m_toolActionIdentifiers);
    m_toolActionIdentifiers.clear();

    AZStd::vector<AZStd::pair<AZStd::string, int>> toolsMenuItems;
    AZStd::vector<AZStd::pair<AZStd::string, int>> toolsToolBarItems;

    // Place all actions in the same sort index in the menu and toolbar.
    // This will display them in order of addition (alphabetical) and ensure no external tool
    // can add items in-between tools without passing through the QtViewPanes system.
    const int sortKey = 0;

    // Get the tools list and refresh the menu.
    const QtViewPanes& viewpanes = m_qtViewPaneManager->GetRegisteredPanes();
    for (const auto& viewpane : viewpanes)
    {
        if (viewpane.IsViewportPane())
        {
            continue;
        }

        AZStd::string toolActionIdentifier = AZStd::string::format("o3de.action.tool.%s", viewpane.m_name.toUtf8().data());

        // Create the action if it does not already exist.
        if (m_actionManagerInterface->GetAction(toolActionIdentifier) == nullptr)
        {
            AzToolsFramework::ActionProperties actionProperties;
            actionProperties.m_name =
                viewpane.m_options.optionalMenuText.length() > 0
                ? viewpane.m_options.optionalMenuText.toUtf8().data()
                : viewpane.m_name.toUtf8().data();
            actionProperties.m_category = "Tool";
            actionProperties.m_iconPath = viewpane.m_options.toolbarIcon;

            m_actionManagerInterface->RegisterCheckableAction(
                EditorMainWindowActionContextIdentifier,
                toolActionIdentifier,
                actionProperties,
                [viewpaneManager = m_qtViewPaneManager, viewpaneName = viewpane.m_name]
                {
                    viewpaneManager->TogglePane(viewpaneName);
                },
                [viewpaneManager = m_qtViewPaneManager, viewpaneName = viewpane.m_name]() -> bool
                {
                    return viewpaneManager->IsVisible(viewpaneName);
                }
            );
        }

        m_toolActionIdentifiers.push_back(toolActionIdentifier);

        if (viewpane.m_options.showInMenu)
        {
            toolsMenuItems.push_back({ toolActionIdentifier, sortKey });
        }

        if (viewpane.m_options.showOnToolsToolbar)
        {
            toolsToolBarItems.push_back({ toolActionIdentifier, sortKey });
        }
    }

    m_menuManagerInterface->AddActionsToMenu(ToolsMenuIdentifier, toolsMenuItems);
    m_toolBarManagerInterface->AddActionsToToolBar(ToolsToolBarIdentifier, toolsToolBarItems);
}