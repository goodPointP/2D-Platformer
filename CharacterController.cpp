//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "CharacterController.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "PlatformerGame.hpp"
#include "SpriteComponent.hpp"

CharacterController::CharacterController(GameObject *gameObject) : Component(gameObject) {
    characterPhysics = gameObject->addComponent<PhysicsComponent>();

    auto physicsScale = PlatformerGame::instance->physicsScale;
    radius = 10/physicsScale;
    characterPhysics->initCircle(b2_dynamicBody, radius, glm::vec2{1.5,1.5}*Level::tileSize/physicsScale,1);
    characterPhysics->getFixture()->SetRestitution(0);
    characterPhysics->fixRotation();
    spriteComponent = gameObject->getComponent<SpriteComponent>();

}

bool CharacterController::onKey(SDL_Event &event) {
        switch (event.key.keysym.sym){
            case SDLK_SPACE:
            {
                if (isGrounded && event.type == SDL_KEYDOWN){ // prevents double jump
                    jump();
                }
            }
            break;
                case SDLK_LEFT:
            {
                left = event.type == SDL_KEYDOWN;
            }
            break;
                case SDLK_RIGHT:
            {
                right = event.type == SDL_KEYDOWN;
            }
            break;
        }

    return false;
}

void CharacterController::update(float deltaTime) {
    // raycast ignores any shape in the starting point
    auto from = characterPhysics->getBody()->GetWorldCenter();
    b2Vec2 to {from.x,from.y -radius*1.3f};
    isGrounded = false;
    PlatformerGame::instance->world->RayCast(this, from, to);

    characterPhysics->fixRotation();
    glm::vec2 movement{0,0};
    if (left){
        movement.x --;
    }
    if (right){
        movement.x ++;
    }
    float accelerationSpeed = 0.010f;
    characterPhysics->addImpulse(movement*accelerationSpeed);
    float maximumVelocity = 2;
    auto linearVelocity = characterPhysics->getLinearVelocity();
    float currentVelocity = linearVelocity.x;
    if (abs(currentVelocity) > maximumVelocity){
        linearVelocity.x = glm::sign(linearVelocity.x)*maximumVelocity;
        characterPhysics->setLinearVelocity(linearVelocity);
    }
    updateSprite(deltaTime);
}

void CharacterController::jump() {
    characterPhysics->addImpulse({0,0.15f});
}

void CharacterController::onCollisionStart(PhysicsComponent *comp) {

}

void CharacterController::onCollisionEnd(PhysicsComponent *comp) {

}

float32 CharacterController::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float32 fraction) {
    isGrounded = true;
    return 0; // terminate raycast
}

void CharacterController::setSprites(sre::Sprite standing, sre::Sprite walk1, sre::Sprite walk2, sre::Sprite flyUp,
                                     sre::Sprite fly, sre::Sprite flyDown) {
    this->standing = standing;
    this->walk1 = walk1;
    this->walk2 = walk2;
    this->flyUp = flyUp;
    this->fly = fly;
    this->flyDown = flyDown;
}

void CharacterController::updateSprite(float deltaTime) {
    auto velocity = characterPhysics->getLinearVelocity();
    
    /*std::cout << velocity.x << std::endl;
    std::cout << deltaTime << std::endl;*/

    // JUMPING
    if (velocity.y > 1) {
        // TO THE LEFT
        if (velocity.x < 0) {
            flyUp.setFlip(glm::bvec2(1, 0));
        }
        // TO THE RIGHT
        else {
            flyUp.setFlip(glm::bvec2(0, 0));
        }
        spriteComponent->setSprite(flyUp);
    }
    // FLYING
    else if (velocity.y <= 1 && velocity.y >= -1 && !isGrounded) {
        if (velocity.x < 0) {
            fly.setFlip(glm::bvec2(1, 0));
        }
        else {
            fly.setFlip(glm::bvec2(0, 0));
        }
        spriteComponent->setSprite(fly);
    }
    // FALLING DOWN
    else if (velocity.y < -1) {
        if (velocity.x < 0) {
            flyDown.setFlip(glm::bvec2(1, 0));
        }
        else {
            flyDown.setFlip(glm::bvec2(0, 0));
        }
        spriteComponent->setSprite(flyDown);
    }
    // WALKING RIGHT
    else if (velocity.x > 0) {
        walk1.setFlip(glm::bvec2(0, 0));
        walk2.setFlip(glm::bvec2(0, 0));
        timeSinceLastFrameChange += deltaTime;
        if (walk1active && timeSinceLastFrameChange > 0.2 / velocity.x) {
            spriteComponent->setSprite(walk2);
            walk1active = false;
            timeSinceLastFrameChange = 0.0;
        }
        else if (!walk1active && timeSinceLastFrameChange > 0.2 / velocity.x){
            spriteComponent->setSprite(walk1);
            walk1active = true;
            timeSinceLastFrameChange = 0.0;
        }
    }
    // WALKING LEFT
    else if (velocity.x < 0) {
        walk1.setFlip(glm::bvec2(-1, 0));
        walk2.setFlip(glm::bvec2(-1, 0));
        timeSinceLastFrameChange += deltaTime;
        if (walk1active && timeSinceLastFrameChange > 0.2 / -velocity.x) {
            spriteComponent->setSprite(walk2);
            walk1active = false;
            timeSinceLastFrameChange = 0.0;
        }
        else if (!walk1active && timeSinceLastFrameChange > 0.2 / -velocity.x) {
            spriteComponent->setSprite(walk1);
            walk1active = true;
            timeSinceLastFrameChange = 0.0;
        }
    }
    // STANDING
    else {
        spriteComponent->setSprite(standing);
    }
    
    // todo implement
}


