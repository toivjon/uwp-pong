#include "pch.h"

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::Gaming::Input;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const& application)
    {
        // activate main window when the application is activated.
        application.Activated([this](auto&&, auto&&) {
            CoreWindow::GetForCurrentThread().Activate();
            });

        // add gamepad into gamepad list when attached.
        Gamepad::GamepadAdded([this](auto&&, auto&& gamepad) {
            mGamepads.insert(gamepad);
            });

        // remove gamepad from gamepad list when detached.
        Gamepad::GamepadRemoved([this](auto&&, auto&& gamepad) {
            mGamepads.erase(gamepad);
            });
    }

    void SetWindow(CoreWindow const& window)
    {
        // stop application execution when window closes.
        window.Closed([this](auto&&, auto&&) {
            mRunning = false;
            });

        // stop rendering when the main window is not visible.
        window.VisibilityChanged([this](auto&&, auto& args) {
            mVisible = args.Visible();
            });
    }

    void Load(hstring const&)
    {
        OutputDebugString(L"Load\n");
    }

    void Run()
    {
        mRunning = true;
        while (mRunning) {
            CoreDispatcher dispatcher = CoreWindow::GetForCurrentThread().Dispatcher();
            if (mVisible) {
                for (auto& gamepad : mGamepads) {
                    GamepadReading reading = gamepad.GetCurrentReading();
                    GamepadButtons buttons = reading.Buttons;
                    if ((buttons & GamepadButtons::A) != (GamepadButtons)0) {
                        OutputDebugString(L"BOOM! A button was pressed!");
                    }
                }
                dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
            else {
                dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
            }
        }
    }

    void Uninitialize()
    {
        // ...
    }
private:
    bool              mRunning;
    bool              mVisible;
    std::set<Gamepad> mGamepads;
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
