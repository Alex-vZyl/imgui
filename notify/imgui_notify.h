#pragma once

#include <vector>
#include <string>

#define NOTIFY_MAX_MSG_LENGTH			4096		// Max message content length
#define NOTIFY_PADDING_X				5.f		// Bottom-left X padding
#define NOTIFY_PADDING_Y				21.f		// Bottom-left Y padding
#define NOTIFY_PADDING_MESSAGE_Y		5.f		// Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME			1000.f		// Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS			3000		// Auto dismiss after X ms (default, applied only of no data provided in constructors)
#define NOTIFY_OPACITY					1.0f		// 0-1 Toast opacity
#define NOTIFY_TOAST_FLAGS				ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing

#define NOTIFY_INLINE					inline
#define NOTIFY_NULL_OR_EMPTY(str)		(!str.size())
#define NOTIFY_FORMAT(fn, format, ...)	if (format) { va_list args; va_start(args, format); fn(format, args, __VA_ARGS__); va_end(args); }

typedef int ImGuiToastType;
typedef int ImGuiToastPhase;
typedef int ImGuiToastPos;

#include <Windows.h>
#include "notify_icons_raw.h"
#include "notify_icons_define.h"
#include "../imgui.h"
#include "../imgui_internal.h"

enum ImGuiToastType_
{
	ImGuiToastType_None,
	ImGuiToastType_Success,
	ImGuiToastType_Warning,
	ImGuiToastType_Error,
	ImGuiToastType_Info,
	ImGuiToastType_COUNT
};

enum ImGuiToastPhase_
{
	ImGuiToastPhase_FadeIn,
	ImGuiToastPhase_Wait,
	ImGuiToastPhase_FadeOut,
	ImGuiToastPhase_Expired,
	ImGuiToastPhase_COUNT
};

enum ImGuiToastPos_
{
	ImGuiToastPos_TopLeft,
	ImGuiToastPos_TopCenter,
	ImGuiToastPos_TopRight,
	ImGuiToastPos_BottomLeft,
	ImGuiToastPos_BottomCenter,
	ImGuiToastPos_BottomRight,
	ImGuiToastPos_Center,
	ImGuiToastPos_COUNT
};

class ImGuiToast
{
private:
	ImGuiToastType	type = ImGuiToastType_None;
	char			title[NOTIFY_MAX_MSG_LENGTH];
	char			content[NOTIFY_MAX_MSG_LENGTH];
	int				dismiss_time = NOTIFY_DEFAULT_DISMISS;
	uint64_t		creation_time = 0;
    unsigned        m_ID = 0;

    // Used to create unique notifications.
    inline static unsigned NotificationCount = 0;

private:

	// Setters
	NOTIFY_INLINE auto set_title(const char* format, va_list args) { vsnprintf(this->title, sizeof(this->title), format, args); }
	NOTIFY_INLINE auto set_content(const char* format, va_list args) { vsnprintf(this->content, sizeof(this->content), format, args); }

public:

	NOTIFY_INLINE auto set_title(const char* format, ...) -> void { NOTIFY_FORMAT(this->set_title, format); }
	NOTIFY_INLINE auto set_content(const char* format, ...) -> void { NOTIFY_FORMAT(this->set_content, format); }
	NOTIFY_INLINE auto set_type(const ImGuiToastType& type) -> void { IM_ASSERT(type < ImGuiToastType_COUNT); this->type = type; };

public:

	// Getters
    NOTIFY_INLINE auto get_title() -> char* { return this->title; };
    NOTIFY_INLINE unsigned get_ID() { return m_ID; }
	NOTIFY_INLINE std::string get_default_title()
	{
		if (!strlen(this->title))
		{
			switch (this->type)
			{
			case ImGuiToastType_None:
				return NULL;
			case ImGuiToastType_Success:
				return "Success";
			case ImGuiToastType_Warning:
				return "Warning";
			case ImGuiToastType_Error:
				return "Error";
			case ImGuiToastType_Info:
				return "Info";
			}
		}
		return this->title;
	};
	NOTIFY_INLINE auto get_type() -> const ImGuiToastType& { return this->type; };
	NOTIFY_INLINE auto get_color() -> const ImVec4&
	{
		switch (this->type)
		{
		case ImGuiToastType_None:
			return { 255, 255, 255, 255 }; // White
		case ImGuiToastType_Success:
			return { 0, 255, 0, 255 }; // Green
		case ImGuiToastType_Warning:
			return { 255, 255, 0, 255 }; // Yellow
		case ImGuiToastType_Error:
			return { 255, 0, 0, 255 }; // Error
		case ImGuiToastType_Info:
			return { 0, 157, 255, 255 }; // Blue
		}
	}
	NOTIFY_INLINE auto get_icon() -> const char*
	{
		switch (this->type)
		{
		case ImGuiToastType_None:
			return NULL;
		case ImGuiToastType_Success:
			return ICON_FA_CHECK_CIRCLE;
		case ImGuiToastType_Warning:
			return ICON_FA_EXCLAMATION_TRIANGLE;
		case ImGuiToastType_Error:
			return ICON_FA_TIMES_CIRCLE;
		case ImGuiToastType_Info:
			return ICON_FA_INFO_CIRCLE;
		}
	}
	NOTIFY_INLINE auto get_content() -> char* { return this->content; };
	NOTIFY_INLINE auto get_elapsed_time() { return GetTickCount64() - this->creation_time; }
	NOTIFY_INLINE auto get_phase() -> const ImGuiToastPhase&
	{
		const auto elapsed = get_elapsed_time();

		if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismiss_time + NOTIFY_FADE_IN_OUT_TIME)
		{
			return ImGuiToastPhase_Expired;
		}
		else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismiss_time)
		{
			return ImGuiToastPhase_FadeOut;
		}
		else if (elapsed > NOTIFY_FADE_IN_OUT_TIME)
		{
			return ImGuiToastPhase_Wait;
		}
		else
		{
			return ImGuiToastPhase_FadeIn;
		}
	}
	NOTIFY_INLINE auto get_fade_percent() -> const float
	{
		const auto phase = get_phase();
		const auto elapsed = get_elapsed_time();
		if (phase == ImGuiToastPhase_FadeIn)
		{
			return ((float)elapsed / (float)NOTIFY_FADE_IN_OUT_TIME) * NOTIFY_OPACITY;
		}
		else if (phase == ImGuiToastPhase_FadeOut)
		{
			return (1.f - (((float)elapsed - (float)NOTIFY_FADE_IN_OUT_TIME - (float)this->dismiss_time) / (float)NOTIFY_FADE_IN_OUT_TIME)) * NOTIFY_OPACITY;
		}
		return 1.f * NOTIFY_OPACITY;
	}

public:

	// Constructors
	ImGuiToast(ImGuiToastType type, int dismiss_time = NOTIFY_DEFAULT_DISMISS)
	{
		IM_ASSERT(type < ImGuiToastType_COUNT);

		this->type = type;
		this->dismiss_time = dismiss_time;
		this->creation_time = GetTickCount64();

		memset(this->title, 0, sizeof(this->title));
		memset(this->content, 0, sizeof(this->content));

        // Assign ID.
        m_ID = ImGuiToast::NotificationCount++;
	}
	ImGuiToast(ImGuiToastType type, const char* format, ...)
        : ImGuiToast(type)
    {
        NOTIFY_FORMAT(this->set_content, format);
    }
	ImGuiToast(ImGuiToastType type, int dismiss_time, const char* format, ...)
        : ImGuiToast(type, dismiss_time)
    {
        NOTIFY_FORMAT(this->set_content, format);
    }
};

namespace ImGui
{
	NOTIFY_INLINE std::vector<ImGuiToast> notifications;

	/// Insert a new toast in the list
	NOTIFY_INLINE void InsertNotification(const ImGuiToast& toast)
	{
		notifications.push_back(toast);
	}

	/// Remove a toast from the list by its index
	NOTIFY_INLINE void RemoveNotification(int index)
	{
		notifications.erase(notifications.begin() + index);
	}

	/// Render toasts, call at the end of your rendering!
	NOTIFY_INLINE void RenderNotifications()
	{
        if (!notifications.size()) return;

        // Notification style.
        PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));

        // Rendering data.
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4 globalTextColor = style.Colors[ImGuiCol_Text];
		auto vp_size = GetMainViewport()->Size;
		float height = 0.f;

        // Render notifications.
		for (auto i = 0; i < notifications.size(); i++)
		{
			auto* current_toast = &notifications[i];

			// Remove toast if expired
			if (current_toast->get_phase() == ImGuiToastPhase_Expired)
			{
				RemoveNotification(i--);
				continue;
			}

			// Get data.
			std::string icon = std::string(current_toast->get_icon());
			std::string title = std::string(current_toast->get_title());
			std::string content = std::string(current_toast->get_content());
			std::string default_title = current_toast->get_default_title();
			float opacity = current_toast->get_fade_percent(); // Get opacity based of the current phase

			// Window rendering
			auto text_color = current_toast->get_color();
			text_color.w = opacity;

			// Generate new unique name for this toast
			char window_name[50];
			sprintf_s(window_name, "##TOAST%d", current_toast->get_ID());

            // Proper fading.
            PushStyleColor(ImGuiCol_Border, {0.f, 0.f, 0.f, opacity});
            PushStyleColor(ImGuiCol_Separator, { 0.f, 0.f, 0.f, opacity });
            PushStyleColor(ImGuiCol_Text, { globalTextColor.x, globalTextColor.y, globalTextColor.z, opacity });
			SetNextWindowBgAlpha(opacity);

            // Begin the notification.
            SetNextWindowPos({vp_size.x - NOTIFY_PADDING_X, vp_size.y - NOTIFY_PADDING_Y - height}, 0, {1.0f, 1.0f});
            Begin(window_name, NULL, NOTIFY_TOAST_FLAGS);
			// Here we render the toast content
			{
                // Wrap text 1/4 screen width.
                PushTextWrapPos(vp_size.x / 4.f);

				bool was_title_rendered = false;

				// If an icon is set
				if (!NOTIFY_NULL_OR_EMPTY(icon))
				{
					//Text(icon); // Render icon text
					TextColored(text_color, icon.c_str());
					was_title_rendered = true;
				}

				// If a title is set
				if (!NOTIFY_NULL_OR_EMPTY(title))
				{
					// If a title and an icon is set, we want to render on same line
					if (!NOTIFY_NULL_OR_EMPTY(icon))
						SameLine();

					Text(title.c_str()); // Render title text
					was_title_rendered = true;
				}

				else if (!NOTIFY_NULL_OR_EMPTY(default_title))
				{
					if (!NOTIFY_NULL_OR_EMPTY(icon))
						SameLine();

					Text(default_title.c_str()); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
					was_title_rendered = true;
				}

				// In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
				if (was_title_rendered && !NOTIFY_NULL_OR_EMPTY(content))
					SetCursorPosY(GetCursorPosY() + 5.f); // Must be a better way to do this!!!!

				// If a content is set
				if (!NOTIFY_NULL_OR_EMPTY(content))
				{
					Separator();
					Text(content.c_str());
				}

                PopTextWrapPos();

		        // Save height for next toasts
		        height += GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;
			}

			// End
         	End();
            PopStyleColor();
            PopStyleColor();
            PopStyleColor();
		}

        // End notification style.
        PopStyleColor();
        PopStyleVar();
	}

	/// Adds font-awesome font, must be called ONCE on initialization
	NOTIFY_INLINE void MergeIconsWithLatestFont(float font_size, bool FontDataOwnedByAtlas = false)
	{
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.FontDataOwnedByAtlas = FontDataOwnedByAtlas;

		GetIO().Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), font_size, &icons_config, icons_ranges);
	}
}
