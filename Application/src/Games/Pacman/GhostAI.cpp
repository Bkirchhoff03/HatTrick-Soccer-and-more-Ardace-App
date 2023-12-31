/*
 * GhostAI.cpp
 *
 *  Created on: Sep 7, 2023
 *      Author: nebk2
 */
#include "GhostAI.h"
#include "Screen.h"
#include "PacmanLevel.h"
#include "Pacman.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include "Color.h"
#include "Circle.h"

namespace {
const uint32_t PEN_WAIT_DURATION = 5000;
const uint32_t SCATTER_MODE_DURATION = 10000;
}

GhostAI::GhostAI() :
		mnoptrGhost(nullptr) {
}

void GhostAI::init(Ghost &ghost, uint32_t lookAheadDistance, const Vec2D &scatterTarget, const Vec2D &ghostPenTarget,
		const Vec2D &ghostExitPenPosition, GhostName name) {
	mGhostPenTarget = ghostPenTarget;
	mGhostExitPenPosition = ghostExitPenPosition;
	mScatterTarget = scatterTarget;
	mLookAheadDistance = lookAheadDistance;
	mnoptrGhost = &ghost;
	mName = name;
	std::random_device r;
	mAIRandomGenerator.seed(r());
	mTimer = 0;
	setState(GHOST_AI_STATE_SCATTER);
	mLastState = GHOST_AI_STATE_SCATTER;
}

PacmanMovement GhostAI::update(uint32_t dt, const Pacman &pacman, const PacmanLevel &level,
		const std::vector<Ghost> &ghosts) {
	if (mnoptrGhost) {

		if (mState == GHOST_AI_STATE_START) {
			return PACMAN_MOVEMENT_NONE;
		}

		if (mState == GHOST_AI_STATE_IN_PEN) {
			mTimer += dt;

			if (mTimer >= PEN_WAIT_DURATION) {
				setState(GHOST_AI_STATE_EXIT_PEN);
			}

			return PACMAN_MOVEMENT_NONE;

		}

		if (mState == GHOST_AI_STATE_GO_TO_PEN && mnoptrGhost->position() == mGhostPenTarget) {
			setState(GHOST_AI_STATE_IN_PEN);

			mnoptrGhost->setGhostState(GHOST_STATE_ALIVE);

			return PACMAN_MOVEMENT_NONE;
		}

		if (mState == GHOST_AI_STATE_EXIT_PEN && mnoptrGhost->position() == mGhostExitPenPosition) {
			setState(GHOST_AI_STATE_SCATTER);
		}

		if (mState == GHOST_AI_STATE_SCATTER) {
			mTimer += dt;
			if (mTimer >= SCATTER_MODE_DURATION) {
				setState(GHOST_AI_STATE_CHASE);
			}
		}

		PacmanMovement currentDirection = mnoptrGhost->getMovementDirection();

		std::vector<PacmanMovement> tempDirections;
		std::vector<PacmanMovement> possibleDirections;

		possibleDirections = getPerpendicularMovements(currentDirection);

		if (currentDirection != PACMAN_MOVEMENT_NONE) {
			possibleDirections.push_back(currentDirection);
		}

		for (const PacmanMovement &pDirection : possibleDirections) {
			tempDirections.push_back(pDirection);
		}

		possibleDirections.clear();

		for (const PacmanMovement &direction : tempDirections) {
			if (!level.willCollide(*mnoptrGhost, *this, direction)) {
				possibleDirections.push_back(direction);
			}
		}

		//assert(possibleDirections.size() >= 1 && "Why can't we go anywhere?");
		if (possibleDirections.size() == 0) {

			std::cout << mName << " can't go anywhere!" << std::endl;
			std::cout << mState << " is the state" << std::endl;
			assert(false && "Why can't we go anywhere?");
		}

		std::sort(possibleDirections.begin(), possibleDirections.end(),
				[](const PacmanMovement &direction1, const PacmanMovement &direction2) {
					return direction1 < direction2;
				});

		if (mnoptrGhost->isVulnerable() && (mState == GHOST_AI_STATE_SCATTER || mState == GHOST_AI_STATE_CHASE)) {
			std::uniform_int_distribution<size_t> distribution(0, possibleDirections.size() - 1);
			return possibleDirections[static_cast<int>(distribution(mAIRandomGenerator))];
		}

		if (mState == GHOST_AI_STATE_CHASE) {
			changeTarget(getChaseTarget(dt, pacman, level, ghosts));
		}

		PacmanMovement directionToGoIn = PACMAN_MOVEMENT_NONE;

		uint32_t lowestDistance = UINT32_MAX;

		for (const PacmanMovement &direction : possibleDirections) {
			Vec2D movementVec = getMovementVector(direction) * mLookAheadDistance;

			AARectangle bbox = mnoptrGhost->getBoundingBox();

			bbox.moveBy(movementVec);

			uint32_t distanceToTarget = bbox.getCenterPoint().distance(mTarget);

			if (distanceToTarget < lowestDistance) {
				directionToGoIn = direction;
				lowestDistance = distanceToTarget;
			}
		}

		assert(directionToGoIn != PACMAN_MOVEMENT_NONE);

		return directionToGoIn;
	}

	return PACMAN_MOVEMENT_NONE;
}
void GhostAI::draw(Screen &screen) {
	if (mnoptrGhost) {
		Circle targetCircle = Circle(mTarget, 4);

		screen.draw(targetCircle, mnoptrGhost->getSpriteColor(), true, mnoptrGhost->getSpriteColor());

		AARectangle bbox = mnoptrGhost->getBoundingBox();

		bbox.moveBy(getMovementVector(mnoptrGhost->getMovementDirection()) * mnoptrGhost->getBoundingBox().getWidth());

		Color c = Color(mnoptrGhost->getSpriteColor().getRed(), mnoptrGhost->getSpriteColor().getGreen(),
				mnoptrGhost->getSpriteColor().getBlue(), 200);
		screen.draw(bbox, mnoptrGhost->getSpriteColor(), true, c);
	}
}

void GhostAI::changeTarget(const Vec2D &target) {
	mTarget = target;
}

Vec2D GhostAI::getChaseTarget(uint32_t dt, const Pacman &pacman, const PacmanLevel &level,
		const std::vector<Ghost> &ghosts) {
	Vec2D target;
	switch (mName) {
	case BLINKY: {
		target = pacman.getBoundingBox().getCenterPoint();
		break;
	}
	case PINKY: {
		target = pacman.getBoundingBox().getCenterPoint()
				+ 2 * getMovementVector(pacman.getMovementDirection()) * pacman.getBoundingBox().getWidth();
		break;
	}
	case INKY: {
		Vec2D pacmanOffsetPoint = pacman.getBoundingBox().getCenterPoint()
				+ (getMovementVector(pacman.getMovementDirection()) * pacman.getBoundingBox().getWidth());
		target = (pacmanOffsetPoint - ghosts[BLINKY].getBoundingBox().getCenterPoint()) * 2
				+ ghosts[BLINKY].getBoundingBox().getCenterPoint();
	}
		break;
	case CLYDE: {
		auto distanceToPacmanBox = mnoptrGhost->getBoundingBox().getCenterPoint().distance(
				pacman.getBoundingBox().getCenterPoint());
		if (distanceToPacmanBox > pacman.getBoundingBox().getWidth() * 4) {
			target = pacman.getBoundingBox().getCenterPoint();
		} else {
			target = mScatterTarget;
		}
	}
		break;
	default: {
		break;
	}
	}
	return target;
}

void GhostAI::setState(GhostAIState state) {
	if (mState == GHOST_AI_STATE_SCATTER || mState == GHOST_AI_STATE_CHASE) {
		mLastState = mState;
	}
	mState = state;

	switch (state) {
	case GHOST_AI_STATE_IN_PEN:
		mTimer = 0;
		break;
	case GHOST_AI_STATE_GO_TO_PEN: {
		Vec2D target = { mGhostPenTarget.GetX() + mnoptrGhost->getBoundingBox().getWidth() / 2, mGhostPenTarget.GetY()
				- mnoptrGhost->getBoundingBox().getHeight() / 2 };
		changeTarget(target);
	}
		break;
	case GHOST_AI_STATE_EXIT_PEN:
		changeTarget(mGhostExitPenPosition);
		break;
	case GHOST_AI_STATE_SCATTER:
		mTimer = 0;
		changeTarget(mScatterTarget);
		break;
	default:
		break;
	}
}

void GhostAI::ghostDelegateGhostStateChangeTo(GhostState lastState, GhostState state) {
	if (mnoptrGhost && mnoptrGhost->isRealeased()
			&& (lastState == GHOST_STATE_VULNERABLE || lastState == GHOST_STATE_VULNERABLE_ENDING)
			&& !(isInPen() || wantsToLeavePen())) {
		mnoptrGhost->setMovementDirection(getOppositeDirection(mnoptrGhost->getMovementDirection()));
	}

	if (state == GHOST_STATE_DEAD) {
		setState(GHOST_AI_STATE_GO_TO_PEN);
	} else if ((lastState == GHOST_STATE_VULNERABLE || lastState == GHOST_STATE_VULNERABLE_ENDING)
			&& state == GHOST_STATE_ALIVE) {
		if (mState == GHOST_AI_STATE_CHASE || mState == GHOST_AI_STATE_SCATTER) {
			setState(mLastState);
		}
	}
}
void GhostAI::ghostWasReleasedFromPen() {
	if (mState == GHOST_AI_STATE_START) {
		setState(GHOST_AI_STATE_EXIT_PEN);
	}
}
void GhostAI::ghostWasResetToFirstPosition() {
	setState(GHOST_AI_STATE_START);
}
