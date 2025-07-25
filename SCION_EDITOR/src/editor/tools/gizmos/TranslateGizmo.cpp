#include "TranslateGizmo.h"
#include "editor/utilities/EditorUtilities.h"
#include "Rendering/Core/BatchRenderer.h"
#include "Rendering/Core/Camera2D.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/Resources/AssetManager.h"
#include "Core/CoreUtilities/CoreUtilities.h"

#include <Rendering/Essentials/Shader.h>
#include <Rendering/Essentials/Texture.h>

#include "Core/ECS/Entity.h"

#include "Logger/Logger.h"

using namespace SCION_CORE::ECS;

namespace SCION_EDITOR
{

TranslateGizmo::TranslateGizmo()
	: Gizmo()
{
	Init( "S2D_x_axis_translate", "S2D_y_axis_translate" );
}

void TranslateGizmo::Update( SCION_CORE::Canvas& canvas )
{
	Gizmo::Update( canvas );

	if ( m_SelectedEntity == entt::null || !m_pRegistry )
	{
		Hide();
		return;
	}

	Show();

	Entity selectedEntity{ *m_pRegistry, m_SelectedEntity };
	auto& selectedTransform = selectedEntity.GetComponent<TransformComponent>();
	float deltaX{ GetDeltaX() };
	float deltaY{ GetDeltaY() };

	bool bTransformChanged = ( deltaX != 0.f || deltaY != 0.f );

	if ( auto* relations = m_pRegistry->GetRegistry().try_get<Relationship>( m_SelectedEntity );
		 relations->parent != entt::null )
	{
		selectedTransform.localPosition.x += deltaX;
		selectedTransform.localPosition.y += deltaY;
	}
	else
	{
		selectedTransform.position.x += deltaX;
		selectedTransform.position.y += deltaY;
	}

	if ( bTransformChanged )
	{
		selectedEntity.UpdateTransform();

		// Update sprite cells
		if ( auto* pSprite = selectedEntity.TryGetComponent<SpriteComponent>())
		{
			if ( pSprite->bIsoMetric )
			{
				auto [ cellX, cellY ] = SCION_CORE::ConvertWorldPosToIsoCoords(
					selectedTransform.position + glm::vec2{ pSprite->width / 2.f, pSprite->height }, canvas );
				pSprite->isoCellX = cellX;
				pSprite->isoCellX = cellY;
			}
		}
	}

	SetGizmoPosition( selectedEntity );

	ExamineMousePosition();
}

void TranslateGizmo::Draw( SCION_RENDERING::Camera2D* pCamera )
{
	if ( m_bHidden )
		return;

	auto pShader = MAIN_REGISTRY().GetAssetManager().GetShader( "basic" );
	if ( !pShader )
		return;

	pShader->Enable();

	glm::mat4 camMat{ 1.f };
	if ( m_bUIComponent && pCamera )
	{
		camMat = pCamera->GetCameraMatrix();
	}
	else
	{
		camMat = m_pCamera->GetCameraMatrix();
	}

	pShader->SetUniformMat4( "uProjection", camMat );

	m_pBatchRenderer->Begin();
	const auto& xAxisSprite = m_pXAxisParams->sprite;
	const auto& xAxisTransform = m_pXAxisParams->transform;

	if ( !xAxisSprite.bHidden )
	{
		glm::vec4 xAxisPosition{ xAxisTransform.position.x,
								 xAxisTransform.position.y,
								 xAxisSprite.width * xAxisTransform.scale.x,
								 xAxisSprite.height * xAxisTransform.scale.y };

		glm::vec4 xAxisUVs{ xAxisSprite.uvs.u, xAxisSprite.uvs.v, xAxisSprite.uvs.uv_width, xAxisSprite.uvs.uv_height };

		const auto pXAxisTexture = MAIN_REGISTRY().GetAssetManager().GetTexture( xAxisSprite.sTextureName );
		if ( pXAxisTexture )
		{
			m_pBatchRenderer->AddSprite(
				xAxisPosition, xAxisUVs, pXAxisTexture->GetID(), 99, glm::mat4{ 1.f }, xAxisSprite.color );
		}
	}

	const auto& yAxisSprite = m_pYAxisParams->sprite;
	const auto& yAxisTransform = m_pYAxisParams->transform;

	if ( !yAxisSprite.bHidden )
	{
		glm::vec4 yAxisPosition{ yAxisTransform.position.x,
								 yAxisTransform.position.y,
								 yAxisSprite.width * yAxisTransform.scale.x,
								 yAxisSprite.height * yAxisTransform.scale.y };

		glm::vec4 yAxisUVs{ yAxisSprite.uvs.u, yAxisSprite.uvs.v, yAxisSprite.uvs.uv_width, yAxisSprite.uvs.uv_height };

		const auto pYAxisTexture = MAIN_REGISTRY().GetAssetManager().GetTexture( yAxisSprite.sTextureName );
		if ( pYAxisTexture )
		{
			m_pBatchRenderer->AddSprite(
				yAxisPosition, yAxisUVs, pYAxisTexture->GetID(), 99, glm::mat4{ 1.f }, yAxisSprite.color );
		}
	}

	m_pBatchRenderer->End();
	m_pBatchRenderer->Render();

	pShader->Disable();
}

} // namespace SCION_EDITOR
