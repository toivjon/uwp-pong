#include "game.h"

using namespace Pong;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Platform;

typedef TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^> ActivatedHandler;
typedef TypedEventHandler<CoreWindow^, CoreWindowEventArgs^> ClosedHandler;
typedef TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^> VisibilityChangedHandler;
typedef TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^> SizeChangedHandler;

ref class View : public IFrameworkView, IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView() {
		return ref new View();
	}

	virtual void Initialize(CoreApplicationView^ view) {
		view->Activated += ref new ActivatedHandler(this, &View::Activated);
	}

	virtual void SetWindow(CoreWindow^ window) {
		window->Closed += ref new ClosedHandler(this, &View::Closed);
		window->VisibilityChanged += ref new VisibilityChangedHandler(this, &View::VisibilityChanged);
		window->SizeChanged += ref new SizeChangedHandler(this, &View::SizeChanged);
	}

	virtual void Load(String^) {
		// ... no operations required
	}

	virtual void Run() {
		game.Run();
	}

	virtual void Uninitialize() {
		// ... no operations required
	}

	void Activated(CoreApplicationView^, IActivatedEventArgs^) {
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void Closed(CoreWindow^, CoreWindowEventArgs^) {
		game.Stop();
	}

	void VisibilityChanged(CoreWindow^, VisibilityChangedEventArgs^ args) {
		if (args->Visible) {
			game.Resume();
		} else {
			game.Pause();
		}
	}

	void SizeChanged(CoreWindow^ window, WindowSizeChangedEventArgs^) {
		// TODO resize game objects and stuff
	}
private:
	Game game;
};

[MTAThread]
int main(Array<String^>^) {
	CoreApplication::Run(ref new View());
	return 0;
}