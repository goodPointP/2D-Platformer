//
// Created by Morten Nobel-Jørgensen on 11/6/17.
//

#include <iostream>
#include "BirdMovementComponent.hpp"
#include "GameObject.hpp"
#include <algorithm>
#include <functional>
#include <glm/glm.hpp>

BirdMovementComponent::BirdMovementComponent(GameObject *gameObject) : Component(gameObject) {}

void BirdMovementComponent::update(float deltaTime) {
    time += deltaTime;
    gameObject->setPosition(computePositionAtTime(time));
}

glm::vec2 BirdMovementComponent::computePositionAtTime(float time) {

    int segment = (int)fmod(time, getNumberOfSegments());
    float t = fmod(time,1.0f);

    std::cout << t << std::endl;


    // todo use Quadratic Bézier spline instead
    /*glm::vec2 P0 = glm::vec2(positions[segment].x, positions[segment].y);
    auto P1 = glm::vec2(positions[segment+1].x, positions[segment+1].y);
    auto P2 = glm::vec2(positions[segment + 2].x, positions[segment + 2].y);*/

    auto xa = getPt(positions[segment + segment].x, positions[segment + 1 + segment].x, t);
    auto ya = getPt(positions[segment + segment].y, positions[segment + 1 + segment].y, t);
    auto xb = getPt(positions[segment + 1 + segment].x, positions[segment + 2 + segment].x, t);
    auto yb = getPt(positions[segment + 1 + segment].y, positions[segment + 2 + segment].y, t);


    // The Black Dot
    auto x = getPt(xa, xb, t);
    auto y = getPt(ya, yb, t);

    return glm::vec2(x,y);

    /*return glm::mix(p, p, t);
    return glm::mix(positions[segment],positions[segment+1],t);*/

}

int BirdMovementComponent::getPt(int n1, int n2, float perc)
{
    int diff = n2 - n1;

    return n1 + (diff * perc);
}

const std::vector<glm::vec2> &BirdMovementComponent::getPositions() {
    return positions;
}

void BirdMovementComponent::setPositions(std::vector<glm::vec2> positions) {
    this->positions = std::move(positions);
}

int BirdMovementComponent::getNumberOfSegments() {
    // todo return number of Quadratic Bézier spline segments instead
    //std::cout << (positions.size() - 1) * 4 - 1<<std::endl;
    return positions.size()/2;
}

