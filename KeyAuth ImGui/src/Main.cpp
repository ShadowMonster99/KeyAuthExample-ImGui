#include <window/Framework.hpp>
#include <auth.hpp>

//harder to hook if its dynamically allocated
bool* loggedin = new bool;

KeyAuth::api KeyAuthApp({

    "", // application name. right above the blurred text aka the secret on the licenses tab among other tabs
    "", // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
    "", // app secret, the blurred text on licenses tab and other tabs
    "1.0", // leave alone unless you've changed version on website
    "https://keyauth.win/api/1.1/", // change if you're self-hosting
    "ssl pin key (optional)" // don't change unless you intend to pin public certificate key. you can get here in the "Pin SHA256" field https://www.ssllabs.com/ssltest/analyze.html?d=keyauth.win&latest. If you do this you need to be aware of when SSL key expires so you can update it
});

static struct KeyAuthInfo {
    char username[100], password[100], license[100];
}Info;

class Renderer
{
public:
    //this function only gets executed once right after ImGui context is not nullptr
    static void Initialize(void)
    {
        using namespace KeyAuth;

        KeyAuthApp.init();
        {
            if (!KeyAuthApp.data.success)
            {
                MessageBoxA(0, KeyAuthApp.data.message.c_str(), "KeyAuth Error", 0);
                exit(0);
            }
        }
    }

    //if the user is logged in, this will render
    static void LoggedIn(void)
    {
        using namespace KeyAuth;

        Center_Text("user information");
        ImGui::Separator();

        ImGui::Text("Welcome back, %s", (char*)KeyAuthApp.data.username.c_str());
        ImGui::Spacing();
        ImGui::Text("ip, %s", (char*)KeyAuthApp.data.ip.c_str());
        ImGui::Text("hwid, %s", (char*)KeyAuthApp.data.hwid.c_str());
        ImGui::Text("create date, %s", (char*)KeyAuthApp.data.createdate.c_str());
        ImGui::Text("last login, %s", (char*)KeyAuthApp.data.lastlogin.c_str());

        ImGui::Text("subscriptions:");
        for (std::string sub : KeyAuthApp.data.subscriptions) ImGui::Text((char*)sub.c_str());

        ImGui::Text("expiry, %s", (char*)KeyAuthApp.data.expiry.c_str());
    }

    //if the user is not logged in, this will render
    static void NotLoggedIn(void)
    {
        using namespace KeyAuth;

        ImGui::Text("Welcome to KeyAuth!");

        ImGui::InputText("Enter a username", Info.username, sizeof Info.username);
        ImGui::InputText("Enter a password", Info.password, sizeof Info.password);
        ImGui::InputText("Enter a license",  Info.license,  sizeof Info.license);

        if (ImGui::Button("login/register"))
        {
            if (strlen(Info.license) > 0)
            {
                if (KeyAuthApp.regstr(Info.username, Info.password, Info.license)) *loggedin = true;
                else MessageBoxA(0, KeyAuthApp.data.message.c_str(), 0, 0);
            }
            else
            {
                if (KeyAuthApp.login(Info.username, Info.password)) *loggedin = true;
                else MessageBoxA(0, KeyAuthApp.data.message.c_str(), 0, 0);
            }
        }
    }
};

class Interface : public Renderer
{
public:
    static void Handler()
    {
        //de-refrence to check its value
        switch (*loggedin)
        {
            case false: Renderer::NotLoggedIn(); break; 
            case true:  Renderer::LoggedIn();    break;
        }
    }
};

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    //loggedin was allocated in memory and may have a value outside a bool
    //so we must set its value properly (false)
    memset(loggedin, false, sizeof loggedin);

    //setup custom window api
    WindowClass::WindowTitle({ (char*)"KeyAuth ImGui Example C++" });
    WindowClass::WindowDimensions(ImVec2({600,450}));

    //create application window
    Application::Create(/*Interface Handler*/ &Interface::Handler, /*Initializer*/ &Renderer::Initialize);

    return 0;
}