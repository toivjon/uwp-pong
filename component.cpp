#include "component.h"
#include "context.h"
#include "entity.h"
#include "util.h"

using namespace pong;

Component::~Component()
{
	SetEntity(nullptr);
}

void Component::SetEntity(std::shared_ptr<Entity> entity)
{
	auto parent = mEntity.lock();
	if (parent && parent != entity) {
		parent->RemoveComponent(shared_from_this());
	}
	mEntity = entity;
	if (entity) {
		entity->AddComponent(shared_from_this());
	}
}

TextComponent::TextComponent(const std::wstring& text) : mText(text)
{
	// ...
}

TextComponent::TextComponent(const std::wstring& text, COMPTR<IDWriteTextFormat> format)
{

}

void TextComponent::Render(RenderingContext& ctx)
{
	D2D1_RECT_F rect{ 0, 0, 800, 50 };
	ctx.GetGraphics().Get2DContext()->DrawText(
		mText.c_str(),
		(UINT32)mText.size(),
		mFormat.Get(),
		D2D1_RECT_F{ 0, 0, 1000, 1000 },
		mBrush.Get()
	);
}

RectangleComponent::RectangleComponent(const D2D1_RECT_F& rect) : mRect(rect)
{
	// ...
}

void RectangleComponent::Render(RenderingContext& ctx)
{
	ctx.GetGraphics().Get2DContext()->FillRectangle(mRect, mBrush.Get());
}