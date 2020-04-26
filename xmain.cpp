using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Platform;

ref class View sealed : public IFrameworkView, IFrameworkViewSource
{
public:
	virtual IFrameworkView^ CreateView() {
		return ref new View();
	}

	virtual void Initialize(CoreApplicationView^ view) {

	}

	virtual void SetWindow(CoreWindow^ window) {

	}

	virtual void Load(String^) {
		// ... no operations required
	}

	virtual void Run() {

	}

	virtual void Uninitialize() {
		// ... no operations required
	}
};

[MTAThread]
int main(Array<String^>^)
{
	CoreApplication::Run(ref new View());
	return 0;
}