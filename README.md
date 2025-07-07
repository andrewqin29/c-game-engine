# Game Design Document

## Section 0: Summary
-   **Working Title:** Debug Dash
-   **Team Members:** Alex, Aaron, Andrew
-   **Concept:** Fight off the coding bugs with your computer science prowess as you jetpack your way through the obstacles and obtain power-ups. This game is intended for players having an intermediate knowledge of C and a desire to test their abilities in an engaging way.

## Section 1: Gameplay
**Main Ideas**
-   **How does your game progress?** Debug Dash is an endless runner where the player's main objective is to achieve the highest score. Gameplay revolves around avoiding coding bug-themed obstacles and collecting tokens to increase the score. Special power-up items also appear periodically. To activate a power-up, the player must fly into it and then correctly solve a C-language debugging puzzle.
-   **What are the win and loss conditions?** The game concludes upon collision with any obstacle (loss condition). The player's performance is measured by the total tokens collected (no explicit win condition, high score = better).
-   **Are there levels?** The game is an endless runner, so gameplay is continuous. However, obstacles may increase in difficulty or speed may increase to make the game harder as it progresses. There are no distinct levels, though.
-   **Are there points?** Yes, the player's score is based on the total tokens collected.

**Mechanics**
-   **Controls:** Space key press: fly up; space key release: fall. Power-up puzzle answers can be selected using mouse clicks or number keys.
-   **Physics:** The player character will be a dynamic body managed by the physics engine, subject to a constant downward gravitational force. Pressing the space bar will apply an upward impulse or force to the character. Collision detection between the player and obstacles, tokens, and power-ups will be handled by the physics engine, to trigger specific events (e.g., game over, score increment, power-up activation).
-   **Game Flow:** The player controls the character who moves upwards with space key presses, constantly affected by gravity. The player navigates by avoiding coding bug-themed obstacles and collecting tokens. When a power-up is collected, the game will pause, and a dialog box will appear, presenting a multiple-choice debugging puzzle. Success grants a temporary ability, while failure can trigger negative consequences. The game ends when the player collides with an obstacle.
-   **Graphics:** Retro computer theme with "bug" enemies/obstacles that appear. The game will use 3 sprites, obstacles, tokens, and power-ups.

## Section 2: Feature Set

| Feature                                                     | Priority | Assigned To |
| :---------------------------------------------------------- | :------- | :---------- |
| Graphics/Assets: 3 sprites, obstacles, tokens, power-ups    | 1        | Aaron       |
| Key detection and sprite movement (just with space key)     | 1        | Andrew      |
| Collision detection                                         | 1        | Alex        |
| Token and point system                                      | 2        | Alex        |
| Scrolling background                                        | 2        | Andrew      |
| Obstacle (Bug) generation and collision                     | 2        | Aaron       |
| Power-up generation and collision                           | 3        | Aaron       |
| Question generation for power-ups                           | 3        | Andrew      |
| Mechanics for power-ups (features that get changed on player) | 3        | Alex        |
| Sound Effects: sounds for token collection, death, power-ups  | 4        | Andrew      |
| Music: music that changes over the course of the game       | 4        | Aaron       |
| Game over screen with score/tokens collected & Restart Logic  | 4        | Alex        |

**Example Power-ups/Obstacles:**
* "Syntax shield" (temporary invincibility)
* "Speed compiler" (move faster)
* "Memory leak" (lose control for 5 seconds if puzzle fails - a negative consequence)

**Debugging Puzzles:** Multiple-choice questions based on sample code or conceptual questions. For instance, 'Which of the following is NOT a common cause of segmentation faults in C? a) Dereferencing a NULL pointer, b) Array out-of-bounds access, c) Incorrect printf format specifier, d) Infinite loop."

## Section 3: Timeline

| Week | Aaron's Tasks         | Alex's Tasks                     | Andrew's Tasks        |
| :--- | :-------------------- | :------------------------------- | :-------------------- |
| 1    | Draw graphics+sprites | Collision detection             | Sprite Movement      |
| 2    | Obstacle generation   | Token and point system          | Scrolling Background |
| 3    | Power-up generation   | Mechanics for power-ups         | Question Generation  |
| 4    | Music                 | Game Over Screen & Restart Logic | Sound Effects        |

## Section 4: Disaster Recovery

* **Prevention:** We'll have regular brief check-ins to monitor progress, understand task dependencies, share skills, and ensure realistic pacing.
* **Addressing Issues:**
    * **Communicatation:** Any member facing delays or roadblocks should immediately inform the other two members. The other members will assist with problem-solving and debugging.
    * **Re-evaluation:** If issues persist, we will re-evaluate task priorities, simplify features, or re-assign tasks within the team.
    * **Scope Management:** As a last resort for significant roadblocks, we will consider reducing the game's scope by simplifying or removing lower-priority features.
    * **Seek External Advice:** If the team continues to be stuck, we will consult TAs or instructors.