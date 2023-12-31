/*
 * Pacman.cpp
 *
 *  Created on: Aug 14, 2023
 *      Author: nebk2
 */

#include "Pacman.h"
#include "Utils.h"

void Pacman::init(const SpriteSheet &spriteSheet, const std::string &animationsPath, const Vec2D &intialPos,
		uint32_t movementSpeed, bool updateSpriteOnMovement) {
	Actor::init(spriteSheet, animationsPath, intialPos, movementSpeed, false);
	resetToFirstAnimation();
	resetScore();
	mIsDying = false;
	resetGhostEatenMultiplier();
}
void Pacman::update(uint32_t dt) {
	if (mIsDying) {
		mSprite.update(dt);
		mIsDying = !isFinishedAnimation();
		return;
	}

	Actor::update(dt);
}
void Pacman::setMovementDirection(PacmanMovement movement) {
	PacmanMovement currentDirection = getMovementDirection();

	if (movement == PACMAN_MOVEMENT_LEFT && currentDirection != PACMAN_MOVEMENT_LEFT) {
		setAnimation("move_left", true);
		resetDelta();
	} else if (movement == PACMAN_MOVEMENT_RIGHT && currentDirection != PACMAN_MOVEMENT_RIGHT) {
		setAnimation("move_right", true);
		resetDelta();
	} else if (movement == PACMAN_MOVEMENT_DOWN && currentDirection != PACMAN_MOVEMENT_DOWN) {
		setAnimation("move_down", true);
		resetDelta();
	} else if (movement == PACMAN_MOVEMENT_UP && currentDirection != PACMAN_MOVEMENT_UP) {
		setAnimation("move_up", true);
		resetDelta();
	}
	Actor::setMovementDirection(movement);
}

void Pacman::resetToFirstAnimation() {
	setAnimation("move_left", true);
	stop();
}
void Pacman::eatenByGhost() {
	setAnimation("death", false);
	mIsDying = true;
	resetGhostEatenMultiplier();
}
void Pacman::resetScore() {
	mScore = 0;
}
void Pacman::ateItem(uint32_t value) {
	addToScore(value);
}
void Pacman::ateGhost(uint32_t value) {
	addToScore(value * mGhostMultiplier);
	mGhostMultiplier *= 2;
}

void Pacman::addToScore(uint32_t value) {
	mScore += value;
}
