//
// Created by Mads Lassen on 24/10/2019.
//

#include "ScriptComponent.hpp"
#include "sre/Resource.hpp"
#include "GameObject.hpp"
#include <iostream>
#include <SDL_events.h>

using namespace sre;
using namespace std;
using namespace sol;

ScriptComponent::ScriptComponent(GameObject *gameObject) : Component(gameObject) {
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::package, sol::lib::math);

    //Incomplete implementation of GameObject
    lua.new_usertype<GameObject>( "GameObject", // the name of the class, as you want it to be used in lua
            // List the member functions you wish to bind:
            // "name_of_item", &class_name::function_or_variable
                //variables
                "name", &GameObject::name,

                //functions
                "setPosition", &GameObject::setPosition,
                "getPosition", &GameObject::getPosition,
                "setRotation", &GameObject::setRotation,
                "getRotation", &GameObject::getRotation
                //TODO: assignment - implement the variables and functions you need from GameObject.
    );

    auto vec2_type = lua.new_usertype<glm::vec2> ("vec2",
                 sol::constructors<glm::vec2(float, float)>(),
                 "x", &glm::vec2::x,
                 "y", &glm::vec2::y
    );

    //Implementation of SDL_KeyboardEvent
    lua.new_usertype<SDL_KeyboardEvent>("SDL_KeyboardEvent", //from SDL_Event.h
                "type", &SDL_KeyboardEvent::type,        /**< ::SDL_KEYDOWN or ::SDL_KEYUP -- SDL_KEYDOWN value is 768 (0x300) see SDL_event.h */
                "timestamp", &SDL_KeyboardEvent::timestamp,   /**< In milliseconds, populated using SDL_GetTicks() */
                "windowID", &SDL_KeyboardEvent::windowID,     /**< The window with keyboard focus, if any */
                "state", &SDL_KeyboardEvent::state,         /**< ::SDL_PRESSED or ::SDL_RELEASED */
                "repeat", &SDL_KeyboardEvent::repeat,        /**< Non-zero if this is a key repeat */
                "keysym", &SDL_KeyboardEvent::keysym
    );

    //Implementation of SDL_Keysym
    lua.new_usertype<SDL_Keysym>("SDL_Keysym", //from SDL_Keyboard.h
                "scancode", &SDL_Keysym::scancode,        /**< SDL physical key code - see ::SDL_Scancode for details */
                "sym", &SDL_Keysym::sym,   /**< SDL virtual key code - see ::SDL_Keycode for details */
                "mod", &SDL_Keysym::mod     /**< current key modifiers */
    );

    //Implementation of getGameObject() from Component base class
    //This returns a GameObject pointer to the GameObject this component is attached to.
    lua.set_function("getGameObject", [&]()->GameObject* {
        return getGameObject();
    });

    //TODO: You may want to extend this with functionality in Lua to Find other game objects in the scene.
}

bool ScriptComponent::init(string luaScript, bool isFilename) {
    if(isFilename)
    {
        //Load as resource.
        luaString = Resource::loadText(luaScript);
    }
    else
    {
        //use luaScript as a lua string directly.
        luaString = luaScript;
    }

    //Load the script
    auto result = lua.script(luaString, [](lua_State* L, sol::protected_function_result pfr) {
        // pfr will contain things that went wrong, for either loading or executing the script
        // Can throw your own custom error
        // You can also just return it, and let the call-site handle the error if necessary.
        return pfr;}
        );     // evaluate lua script

    //First check whether the script has been loaded correctly
    if(!result.valid())
    {
        isLoaded = false;
        //Handle error has occured
        cerr << "FAILED TO LOAD LUA SCRIPT: " << luaString << "\n";

        //Drop out of the init function, to allow the ScriptComponent to work, until a functional lua script is loaded.
        //This is useful if you're editing your script live.
        return false;
    }

    lua_update = lua["update"];    // get lua function update pos
    lua_onKeyDown = lua["onKeyDown"];    // get lua function update pos
    lua_onKeyUp = lua["onKeyUp"];    // get lua function update pos

    //Successfully loaded the script.
    isLoaded = true;
    return true;
}

bool ScriptComponent::onKey(SDL_Event &event) {
    //Handle keyboard events through Lua
    //First check if lua_onKeyDown has been loaded correctly
    if(event.type == SDL_KEYDOWN && lua_onKeyDown.valid())
    {
        //Then execute lua_onKeyDown
        auto result = lua_onKeyDown(&event.key);
        //And check whether the execution went well
        if(!result.valid())
        {
            //Handle error has occured
            sol::error err = result;
            std::string what = err.what();
            std::cerr << what << std::endl;
        }
        //Returns whether the key event was handled bby lua.
        return result;
    }
    //Check if lua_onKeyUp has been loaded correctly
    else if(event.type == SDL_KEYUP && lua_onKeyUp.valid())
    {
        //Then execute lua_onKeyDown
        auto result = lua_onKeyUp(&event.key);
        //And check whether the execution went well
        if(!result.valid())
        {
            //Handle error has occured
            sol::error err = result;
            std::string what = err.what();
            std::cerr << what << std::endl;
        }
        //Returns whether the key event was handled bby lua.
        return result;
    }

    //We did not handle the key.
    return false;
}

void ScriptComponent::update(float deltaTime) {
    //Handle update through Lua
    //Check if lua_update has been loaded correctly
    if(lua_update.valid())
    {
        //Then execute lua_update
        auto result = lua_update(deltaTime);
        //And check whether the execution went well
        if(!result.valid())
        {
            //Handle error has occured
            sol::error err = result;
            std::string what = err.what();
            std::cerr << what << std::endl;
        }
    }
}

/*
void ScriptComponent::renderSprite(SpriteBatch::SpriteBatchBuilder& spriteBatchBuilder) {
    //TODO: handle rendering through Lua
}

void ScriptComponent::onCollisionStart(PhysicsComponent* comp) {
    //TODO: handle collision events through Lua
}

void ScriptComponent::onCollisionEnd(PhysicsComponent* comp) {
    //TODO: handle collision events through Lua
}
*/

