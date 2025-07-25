#include "PackageDisplay.h"
#include "Core/CoreUtilities/ProjectInfo.h"
#include "Core/CoreUtilities/CoreEngineData.h"
#include "Core/ECS/MainRegistry.h"
#include "ScionUtilities/HelperUtilities.h"
#include "editor/utilities/imgui/ImGuiUtils.h"
#include "editor/utilities/EditorUtilities.h"
#include "editor/scene/SceneManager.h"
#include "ScionFilesystem/Dialogs/FileDialog.h"
#include "Logger/Logger.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace SCION_EDITOR
{
PackageGameDisplay::PackageGameDisplay()
	: m_pGameConfig{ std::make_unique<SCION_CORE::GameConfig>() }
	, m_sDestinationPath{}
	, m_sScriptListPath{}
	, m_sFileIconPath{}
	, m_bResizable{ false }
	, m_bBorderless{ false }
	, m_bFullScreen{ false }
	, m_bTitlebar{ false }
	, m_bScriptListExists{ false }
{
	const auto& pProjectInfo = MAIN_REGISTRY().GetContext<SCION_CORE::ProjectInfoPtr>();
	auto optScriptListPath = pProjectInfo->GetScriptListPath();
	SCION_ASSERT( optScriptListPath && "Script List path not set correctly in project info." );

	m_sScriptListPath = optScriptListPath->string();
	m_bScriptListExists = fs::exists( *optScriptListPath );
}

PackageGameDisplay::~PackageGameDisplay() = default;

void PackageGameDisplay::Update()
{
	// TODO: Handle Packager
}

void PackageGameDisplay::Draw()
{
	if ( !ImGui::Begin( "Package Game" ) )
	{
		ImGui::End();
		return;
	}

	auto& pProjectInfo = MAIN_REGISTRY().GetContext<SCION_CORE::ProjectInfoPtr>();

	ImGui::SeparatorText( "Package and Export Game" );
	ImGui::NewLine();

	if ( ImGui::BeginChild( "Game Packager" ) )
	{
		ImGui::SeparatorText( "File Information" );
		ImGui::PushItemWidth( 256.f );
		ImGui::InlineLabel( "Game Title" );
		std::string sProjectName{ pProjectInfo->GetProjectName() };
		ImGui::InputTextReadOnly( "##gameTitle", &sProjectName );

		static bool bDestinationError{ false };
		ImGui::InlineLabel( "Destination" );
		ImGui::InputTextReadOnly( "##destination", &m_sDestinationPath );
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if ( ImGui::Button( "..."
							"##dest" ) )
		{
			SCION_FILESYSTEM::FileDialog fd{};
			const auto sFilepath = fd.SelectFolderDialog( "Choose Destination Folder", BASE_PATH );
			if ( !sFilepath.empty() )
			{
				if ( !IsReservedPathOrFile( fs::path{ sFilepath } ) )
				{
					m_sDestinationPath = sFilepath;
					bDestinationError = false;
				}
				else
				{
					SCION_ERROR( "Failed to set destination. "
								 "Destination [{}] is a reserved path. "
								 "Please select a different path.",
								 sFilepath );

					bDestinationError = true;
				}
			}
			else
			{
				bDestinationError = false;
			}
		}

		if ( bDestinationError )
		{
			ImGui::TextColored( ImVec4{ 1.f, 0.f, 0.f, 1.f },
								"Invalid Destination. Destinations cannot be reserved paths." );
		}

		ImGui::InlineLabel( "Package Assets" );
		ImGui::ItemToolTip( "Convert assets into luac files and add them to zip archive." );
		ImGui::Checkbox( "##packageassets", &m_pGameConfig->bPackageAssets );
		ImGui::AddSpaces( 2 );
		ImGui::Separator();
		ImGui::AddSpaces( 3 );

		ImGui::SeparatorText( "Default Window Parameters" );
		ImGui::AddSpaces( 2 );
		ImGui::InlineLabel( "Window Size" );
		ImGui::PushItemWidth( 128.f );
		ImGui::InputInt( "##windowWidth", &m_pGameConfig->windowWidth );
		ImGui::SameLine();
		ImGui::InputInt( "##windowHeight", &m_pGameConfig->windowHeight );
		ImGui::PopItemWidth();
		ImGui::Separator();
		ImGui::AddSpaces( 2 );
		ImGui::Text( "Window Flags" );
		ImGui::Separator();

		ImGui::InlineLabel( "Resizable" );
		if ( ImGui::Checkbox( "##resizable", &m_bResizable ) )
		{
			if ( m_bResizable )
			{
				m_bFullScreen = false;
			}
		}

		ImGui::InlineLabel( "Fullscreen" );
		if ( ImGui::Checkbox( "##fullscreen", &m_bFullScreen ) )
		{
			if ( m_bFullScreen )
			{
				m_bResizable = false;
				m_bTitlebar = false;
			}
		}

		ImGui::InlineLabel( "Titlebar" );
		if ( ImGui::Checkbox( "##titlebar", &m_bTitlebar ) )
		{
			if ( m_bTitlebar )
			{
				m_bFullScreen = false;
				m_bBorderless = false;
			}
		}

		ImGui::InlineLabel( "Borderless" );
		if ( ImGui::Checkbox( "##borderless", &m_bBorderless ) )
		{
			if ( m_bBorderless )
			{
				m_bTitlebar = false;
			}
		}

		ImGui::Separator();
		ImGui::AddSpaces( 3 );
		ImGui::SeparatorText( "Default Camera Parameters" );
		ImGui::AddSpaces( 3 );

		ImGui::InlineLabel( "Camera Size" );
		ImGui::PushItemWidth( 128.f );
		ImGui::InputInt( "##cameraWidth", &m_pGameConfig->cameraWidth );
		ImGui::SameLine();
		ImGui::InputInt( "##cameraHeight", &m_pGameConfig->cameraHeight );

		ImGui::InlineLabel( "Camera Scale" );
		ImGui::InputFloat( "##camerascale", &m_pGameConfig->cameraScale );
		ImGui::PopItemWidth();
		ImGui::AddSpaces( 3 );

		ImGui::SeparatorText( "Startup Options" );

		ImGui::PushItemWidth( 256.f );

		if ( ImGui::BeginCombo( "##start_up_scenes", m_pGameConfig->sStartupScene.c_str() ) )
		{
			for ( const auto& sSceneName : SCENE_MANAGER().GetSceneNames() )
			{
				if ( ImGui::Selectable( sSceneName.c_str(), sSceneName == m_pGameConfig->sStartupScene ) )
				{
					m_pGameConfig->sStartupScene = sSceneName;
				}
			}
			ImGui::EndCombo();
		}

		ImGui::PopItemWidth();

		ImGui::EndChild();
	}

	ImGui::Separator();

	ImGui::AddSpaces( 3 );

	if ( ImGui::Button( "Package Game" ) )
	{
	}

	ImGui::End();
}

} // namespace SCION_EDITOR
