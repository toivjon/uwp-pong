#pragma once

#include <d2d1_3.h>
#include <dwrite.h>
#include <memory>
#include <string>
#include <wrl.h>

#ifndef COMPTR
#define COMPTR Microsoft::WRL::ComPtr
#endif

namespace pong
{
	// forward declarations
	class Entity;
	class Graphics;
	class RenderingContext;

	// ========================================================================

	class Component : public std::enable_shared_from_this<Component>
	{
	public:
		Component() = default;
		Component(Component&) = delete;
		Component(Component&&) = delete;

		Component& operator=(Component&) = delete;
		Component& operator=(Component&&) = delete;

		virtual ~Component();

		void SetEntity(std::shared_ptr<Entity> entity);
		std::weak_ptr<Entity> GetEntity() { return mEntity;  }
	private:
		std::weak_ptr<Entity> mEntity;
	};

	// ========================================================================

	class RenderableComponent : public Component
	{
	public:
		RenderableComponent() = default;
		RenderableComponent(RenderableComponent&) = delete;
		RenderableComponent(RenderableComponent&&) = delete;

		RenderableComponent& operator=(RenderableComponent&) = delete;
		RenderableComponent& operator=(RenderableComponent&&) = delete;

		virtual ~RenderableComponent() = default;

		virtual void Render(RenderingContext& ctx) = 0;
		 
		void SetBrush(COMPTR<ID2D1SolidColorBrush> brush)	{ mBrush = brush;	}
		COMPTR<ID2D1SolidColorBrush> GetBrush()				{ return mBrush;	}
	protected:
		COMPTR<ID2D1SolidColorBrush> mBrush;
	};

	// ========================================================================

	class TextComponent final : public RenderableComponent
	{
	public:
		TextComponent() = default;
		TextComponent(TextComponent&) = delete;
		TextComponent(TextComponent&&) = delete;

		TextComponent(const std::wstring& text);
		TextComponent(const std::wstring& text, COMPTR<IDWriteTextFormat> format);

		TextComponent& operator=(TextComponent&) = delete;
		TextComponent& operator=(TextComponent&&) = delete;

		virtual ~TextComponent() = default;

		void Render(RenderingContext& ctx) override;

		void SetText(const std::wstring& text)	{ mText = text; }
		const std::wstring& GetText() const		{ return mText; }
			  std::wstring& GetText()			{ return mText; }

		void SetFormat(COMPTR<IDWriteTextFormat> format) { mFormat = format; }
		COMPTR<IDWriteTextFormat> GetFormat()		 	 { return mFormat;	 }
	private:
		std::wstring mText;
		COMPTR<IDWriteTextFormat> mFormat;
	};

	// ========================================================================

	class RectangleComponent final : public RenderableComponent
	{
	public:
		RectangleComponent() = default;
		RectangleComponent(RectangleComponent&) = delete;
		RectangleComponent(RectangleComponent&&) = delete;

		RectangleComponent(const D2D1_RECT_F& rect);

		RectangleComponent& operator=(RectangleComponent&) = delete;
		RectangleComponent& operator=(RectangleComponent&&) = delete;

		virtual ~RectangleComponent() = default;

		void Render(RenderingContext& ctx) override;

		void SetRect(const D2D1_RECT_F& rect)	{ mRect = rect; }
		const D2D1_RECT_F& GetRect() const		{ return mRect; }
			  D2D1_RECT_F& GetRect()			{ return mRect; }
	private:
		D2D1_RECT_F mRect;
	};

	// ========================================================================
}
