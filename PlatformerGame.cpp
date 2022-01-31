#include <sre/Inspector.hpp>
#include "PlatformerGame.hpp"
#include "GameObject.hpp"
#include "sre/RenderPass.hpp"
#include "sre/Texture.hpp"
#include "SpriteComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "Box2D/Dynamics/Contacts/b2Contact.h"
#include "PhysicsComponent.hpp"
#include "CharacterController.hpp"
#include "BirdMovementComponent.hpp"
#include "ScriptComponent.hpp"

using namespace std;
using namespace sre;

const glm::vec2 PlatformerGame::windowSize(800,600);

PlatformerGame* PlatformerGame::instance = nullptr;

PlatformerGame::PlatformerGame()
        :debugDraw(physicsScale)
{
    instance = this;
    r.setWindowSize(windowSize);
    bool useVsync = true;
    r.init().withSdlInitFlags(SDL_INIT_EVERYTHING)
            .withSdlWindowFlags(SDL_WINDOW_OPENGL)
            .withVSync(useVsync);

    backgroundColor = {0.6f,0.6f,1.0f,1.0f};

    //spriteAtlas = SpriteAtlas::create("platformer-art-deluxe.json","platformer-art-deluxe.png");
    spriteAtlas = SpriteAtlas::create("platformer-art-deluxe.json",Texture::create()
            .withFile( "platformer-art-deluxe.png")
            .withFilterSampling(false)
            .build());

    level = Level::createDefaultLevel(this, spriteAtlas);

    initLevel();

    // setup callback functions
    r.keyEvent = [&](SDL_Event& e){
        onKey(e);
    };
    r.frameUpdate = [&](float deltaTime){
        update(deltaTime);
    };
    r.frameRender = [&](){
        render();
    };
    // start game loop
    r.startEventLoop();
}

void PlatformerGame::initLevel() {
    initPhysics();

    auto player = createGameObject();
    player->name = "Player";
    auto playerSprite = player->addComponent<SpriteComponent>();
    auto playerSpriteObj = spriteAtlas->get("19.png");
    playerSpriteObj.setPosition(glm::vec2{1.5,2.5}*Level::tileSize);
    playerSprite->setSprite(playerSpriteObj);
    auto characterController = player->addComponent<CharacterController>();
    characterController->setSprites(
            spriteAtlas->get("19.png"),
            spriteAtlas->get("20.png"),
            spriteAtlas->get("21.png"),
            spriteAtlas->get("26.png"),
            spriteAtlas->get("27.png"),
            spriteAtlas->get("28.png")
    );

    auto camObj = createGameObject();
    camObj->name = "Camera";
    camera = camObj->addComponent<SideScrollingCamera>();
    camObj->setPosition(windowSize*0.5f);
    camera->setFollowObject(player,{200,windowSize.y*0.5f});

    auto birdObj = createGameObject();
    birdObj->name = "Bird";
    auto spriteComponent = birdObj->addComponent<SpriteComponent>();
    auto bird = spriteAtlas->get("433.png");
    bird.setFlip({true,false});
    spriteComponent->setSprite(bird);
    birdMovement = birdObj->addComponent<BirdMovementComponent>().get();

    birdMovement->setPositions({
                                       {-50,350},
                                       {0,300},
                                       {50,350},
                                       {100,400},
                                       {150,300},
                                       {200,200},
                                       {250,300},
                                       {300,400},
                                       {350,350},
                                       {400,300},
                                       {450,350},
                                       {500,400},
                                       {550,350},
                                       {600,300},
                                       {650,350},
                                       {700,400},
                                       {750,350},
                                       {800,300},
                                       {850,350},
                                       {900,400},
                                       {950,350},
                                       {1000,300},
                                       {1050,350},
                                       {1100,400},
                                       {1150,350},
                                       {1200,300},
                                       {1250,350},
                               });

    auto spiralBirdObj = createGameObject();
    spiralBirdObj->name = "spiralBird";
    auto sBirdSpriteComponent = spiralBirdObj->addComponent<SpriteComponent>();
    auto sBird = spriteAtlas->get("433.png");
    sBird.setFlip({true,false});
    sBirdSpriteComponent->setSprite(sBird);
    spiralBirdObj->setPosition(glm::vec2(100, 200));

    //TODO: assignment - Uncomment and add this ScriptComponent for exercise 10-4 and extend the ScriptComponent with the relevant functions from GameObject.
     //REMOVE THIS BLOCK COMMENT
    auto script = spiralBirdObj->addComponent<ScriptComponent>();

     // TODO: assignment - update radius continuously in the lua script so that the bird flies in a spiral with radius between [10, 100]
    std::string luaScript =
            "radius = 100\n"
            "increasing = false\n"
            "-- Function evaluated for each update\n"
            "function update (time) \n"
            "   if (radius > 10 and not increasing or radius==100) then \n"
            "       radius = radius - 1 \n"
            "       increasing = false \n"
            "   elseif (radius == 10 or increasing) then \n"
            "       radius = radius + 1 \n"
            "       increasing = true \n"
            "   end\n"
            "   gameobj = getGameObject() \n"
            "   rot = gameobj:getRotation()\n"
            "   rot = (rot + 250 * time ) % (360) \n"
            "   gameobj:setRotation(rot) \n"
            "   pos = { x = 300, y = 400} \n"
            "   pos.x = pos.x + math.sin(math.pi/180 * rot) * radius \n"
            "   pos.y = pos.y - math.cos(math.pi/180 * rot) * radius \n"
            "   gameobj:setPosition(vec2.new(pos.x, pos.y)) \n"
            "end\n"
            "-- Key down handler example \n"
            "function onKeyDown (key) \n"
            "   --NOTE: this simple char comparison does not handle special keys.\n"
            "   --Keycode values can be found in https://wiki.libsdl.org/SDLKeycodeLookup here using ('-') which is SDLK_MINUS = 45\n"
            "   if(key.keysym.sym == 45) then \n"
            "       print(\"pressed!!!\")\n"
            "       return true \n"
            "   end\n"
            "   --result was not consumed.\n"
            "   return false \n"
            "end\n";

    script->init(luaScript);
    

    level->generateLevel();
}

void PlatformerGame::update(float time) {
    updatePhysics();
	if (time > 0.03) // if framerate approx 30 fps then run two physics steps
	{
		updatePhysics();
	}
    for (int i=0;i<sceneObjects.size();i++){
        sceneObjects[i]->update(time);
    }
}

void PlatformerGame::render() {
    auto rp = RenderPass::create()
            .withCamera(camera->getCamera())
            .withClearColor(true, backgroundColor)
            .build();

    if (doDebugDraw){
        static Inspector profiler;
        profiler.update();
        profiler.gui(false);

        std::vector<glm::vec3> lines;
        for (int i=0;i<5000;i++){
            float t = (i/5001.0f)*birdMovement->getNumberOfSegments();
            float t1 = ((i+1)/5001.0f)*birdMovement->getNumberOfSegments();
            auto p = birdMovement->computePositionAtTime(t);
            auto p1 = birdMovement->computePositionAtTime(t1);
            lines.push_back(glm::vec3(p,0));
            lines.push_back(glm::vec3(p1,0));
        }
        rp.drawLines(lines);
    }

    auto pos = camera->getGameObject()->getPosition();

    auto spriteBatchBuilder = SpriteBatch::create();
    for (auto & go : sceneObjects){
        go->renderSprite(spriteBatchBuilder);
    }

    auto sb = spriteBatchBuilder.build();
    rp.draw(sb);

    if (doDebugDraw){
        world->DrawDebugData();
        rp.drawLines(debugDraw.getLines());
        debugDraw.clear();
    }
}

void PlatformerGame::onKey(SDL_Event &event) {
    for (auto & gameObject: sceneObjects) {
        for (auto & c : gameObject->getComponents()){
            bool consumed = c->onKey(event);
            if (consumed){
                return;
            }
        }
    }

    if (event.type == SDL_KEYDOWN){
        switch (event.key.keysym.sym){
            case SDLK_z:
                camera->setZoomMode(!camera->isZoomMode());
                break;
            case SDLK_d:
                // press 'd' for physics debug
                doDebugDraw = !doDebugDraw;
                if (doDebugDraw){
                    world->SetDebugDraw(&debugDraw);
                } else {
                    world->SetDebugDraw(nullptr);
                }
                break;
        }
    }
}

std::shared_ptr<GameObject> PlatformerGame::createGameObject() {
    auto obj = shared_ptr<GameObject>(new GameObject());
    sceneObjects.push_back(obj);
    return obj;
}

void PlatformerGame::updatePhysics() {

    const int positionIterations = 4;
    const int velocityIterations = 12;
    world->Step(timeStep, velocityIterations, positionIterations);

    for (auto phys : physicsComponentLookup){
        PhysicsComponent* physicsComponent = phys.second;
        if (physicsComponent->isAutoUpdate() == false) continue;
        auto position = physicsComponent->getBody()->GetPosition();
        float angle = physicsComponent->getBody()->GetAngle();
        auto gameObject = physicsComponent->getGameObject();
        gameObject->setPosition(glm::vec2(position.x*physicsScale, position.y*physicsScale));
        gameObject->setRotation(angle);
    }
}

void PlatformerGame::initPhysics() {
    float gravity = -9.8f; // 9.8 m/s2
    delete world;
    world = new b2World(b2Vec2(0,gravity));
    world->SetContactListener(this);

    if (doDebugDraw){
        world->SetDebugDraw(&debugDraw);
    }
}

void PlatformerGame::BeginContact(b2Contact *contact) {
    b2ContactListener::BeginContact(contact);
    handleContact(contact, true);
}

void PlatformerGame::EndContact(b2Contact *contact) {
    b2ContactListener::EndContact(contact);
    handleContact(contact, false);
}

void PlatformerGame::deregisterPhysicsComponent(PhysicsComponent *r) {
    auto iter = physicsComponentLookup.find(r->getFixture());
    if (iter != physicsComponentLookup.end()){
        physicsComponentLookup.erase(iter);
    } else {
        assert(false); // cannot find physics object
    }
}

void PlatformerGame::registerPhysicsComponent(PhysicsComponent *r) {
    physicsComponentLookup[r->getFixture()] = r;
}

void PlatformerGame::handleContact(b2Contact *contact, bool begin) {
    auto fixA = contact->GetFixtureA();
    auto fixB = contact->GetFixtureB();
    auto physA = physicsComponentLookup.find(fixA);
    auto physB = physicsComponentLookup.find(fixB);
    if (physA !=physicsComponentLookup.end() && physB != physicsComponentLookup.end()){
        auto & aComponents = physA->second->getGameObject()->getComponents();
        auto & bComponents = physB->second->getGameObject()->getComponents();
        for (auto & c : aComponents){
            if (begin){
                c->onCollisionStart(physB->second);
            } else {
                c->onCollisionEnd(physB->second);
            }
        }
        for (auto & c : bComponents){
            if (begin){
                c->onCollisionStart(physA->second);
            } else {
                c->onCollisionEnd(physA->second);
            }
        }
    }
}
