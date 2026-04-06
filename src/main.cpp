#include "App.hpp"
#include "Core/Context.hpp"

int main(int, char**) {
    auto context = Core::Context::GetInstance();
    App app;

    app.Start();

    while (!context->GetExit()) {

        if (app.GetCurrentState() != App::State::END) {
            app.Update();
        } else {
            app.End();
            context->SetExit(true);
        }

        context->Update();
    }
    return 0;
}