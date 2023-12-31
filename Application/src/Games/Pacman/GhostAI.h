/*
 * GhostAI.h
 *
 *  Created on: Sep 7, 2023
 *      Author: nebk2
 */

#ifndef GAMES_PACMAN_GHOSTAI_H_
#define GAMES_PACMAN_GHOSTAI_H_

#include "Ghost.h"
#include "PacmanGameUtils.h"
#include <random>
#include <stdint.h>
#include <vector>

class PacmanLevel;
class Screen;
class Pacman;

enum GhostAIState {
	GHOST_AI_STATE_START = 0,
	GHOST_AI_STATE_IN_PEN,
	GHOST_AI_STATE_EXIT_PEN,
	GHOST_AI_STATE_CHASE,
	GHOST_AI_STATE_SCATTER,
	GHOST_AI_STATE_GO_TO_PEN
};

class GhostAI: public GhostDelegate {
public:
	GhostAI();
	void init(Ghost &ghost, uint32_t lookAheadDistance, const Vec2D &scatterTarget, const Vec2D &ghostPenTarget,
			const Vec2D &ghostExitPenPosition, GhostName name);

	PacmanMovement update(uint32_t dt, const Pacman &pacman, const PacmanLevel &level,
			const std::vector<Ghost> &ghosts);
	void draw(Screen &screen);

	inline bool wantsToLeavePen() const {
		return mState == GHOST_AI_STATE_EXIT_PEN;
	}
	inline bool isInPen() const {
		return mState == GHOST_AI_STATE_IN_PEN || mState == GHOST_AI_STATE_START;
	}
	inline bool isEnteringPen() const {
		return mState == GHOST_AI_STATE_GO_TO_PEN;
	}

	virtual void ghostDelegateGhostStateChangeTo(GhostState lastState, GhostState state) override;
	virtual void ghostWasReleasedFromPen() override;
	virtual void ghostWasResetToFirstPosition() override;

private:

	void setState(GhostAIState state);
	void changeTarget(const Vec2D &target);
	Vec2D getChaseTarget(uint32_t dt, const Pacman &pacman, const PacmanLevel &level, const std::vector<Ghost> &ghosts);

	Vec2D mGhostExitPenPosition;
	Vec2D mGhostPenTarget;
	Vec2D mScatterTarget;
	Vec2D mTarget;
	uint32_t mLookAheadDistance;
	Ghost *mnoptrGhost;
	GhostName mName;
	std::default_random_engine mAIRandomGenerator;
	GhostAIState mState;
	GhostAIState mLastState;
	uint64_t mTimer;
};

#endif /* GAMES_PACMAN_GHOSTAI_H_ */
