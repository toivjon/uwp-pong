using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Platform;

ref class View : public IFrameworkView, IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView() {
		return ref new View();
	}

	virtual void Initialize(CoreApplicationView^ view) {
		view->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &View::Activated);
	}

	virtual void SetWindow(CoreWindow^ window) {
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &View::WindowClosed);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &View::WindowVisibilityChanged);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &View::WindowSizeChanged);
	}

	virtual void Load(String^) {
		// ... no operations required
	}

	virtual void Run() {

	}

	virtual void Uninitialize() {
		// ... no operations required
	}

	void Activated(CoreApplicationView^, IActivatedEventArgs^) {
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void WindowClosed(CoreWindow^, CoreWindowEventArgs^) {
		// TODO stop game
	}

	void WindowVisibilityChanged(CoreWindow^, VisibilityChangedEventArgs^ args) {
		// TODO pause/unpause game
	}

	void WindowSizeChanged(CoreWindow^ window, WindowSizeChangedEventArgs^) {
		// TODO resize game objects and stuff
	}
};

[MTAThread]
int main(Array<String^>^) {
	CoreApplication::Run(ref new View());
	return 0;
}