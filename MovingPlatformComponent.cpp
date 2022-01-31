//
// Created by Morten Nobel-Jørgensen on 11/6/17.
//

#include "MovingPlatformComponent.hpp"
#include "PlatformComponent.hpp"
#include "GameObject.hpp"
#include <iostream>

MovingPlatformComponent::MovingPlatformComponent(GameObject *gameObject) : Component(gameObject)
{
    platformComponent = gameObject->getComponent<PlatformComponent>();
}

void MovingPlatformComponent::update(float deltaTime) {
    totalTime += deltaTime;

    // todo replace with easing function
    /*if (fmod(totalTime,2)>1){
        platformComponent->moveTo(movementEnd);
    } else {
        platformComponent->moveTo(movementStart);
    }*/
   
    if (platformComponent->getPosition().y >= 115.5 || platformComponent->getPosition().y <= 73.5) {
        increasingH = !increasingH;
    }

    if (increasingH) {
        movementCurrent = movementCurrent + glm::vec2(0, sin(totalTime)/20);
    }
    else {
        movementCurrent = movementCurrent - glm::vec2(0, sin(totalTime)/20);
    }

    platformComponent->moveTo(movementCurrent);

    if (totalTime >= 4) {
        totalTime = 0;
    }
}

void MovingPlatformComponent::setMovementStart(glm::vec2 pos) {
    movementStart = pos;
}

void MovingPlatformComponent::setMovementEnd(glm::vec2 pos) {
    movementEnd = pos;
}
