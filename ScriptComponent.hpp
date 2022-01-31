//
// Created by Mads Lassen on 24/10/2019.
//

#pragma once

#include "Component.hpp"
#include "sol.hpp"
#include <SDL_events.h>

class ScriptComponent : public Component {
public:
    explicit ScriptComponent(GameObject *gameObject);

    bool init(std::string luaScript, bool isFilename = false);

    // The function should return true, if the key event is consumed. This prevents other components to receive the event.
    bool onKey(SDL_Event &event) override;

    void update(float deltaTime) override;
    /*
    void renderSprite(sre::SpriteBatch::SpriteBatchBuilder& spriteBatchBuilder) override;

    void onCollisionStart(PhysicsComponent* comp) override;
    void onCollisionEnd(PhysicsComponent* comp) override;
     */

private:
    std::string luaString;
    bool isLoaded = false;

    std::string error;
    sol::state lua;
    sol::protected_function lua_update;
    sol::protected_function lua_onKeyDown;
    sol::protected_function lua_onKeyUp;
};
