#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * This constant is used to improve the program speed: it will be used to drop
 * all states which couldn't win under the required time.
 * It's also used to improve the memory usage, since inputs are pre-allocated in
 * memory.
 */
#define MAX_FRAMES 167

#define INITIAL_GEAR 0
#define INITIAL_SPEED 0

#define INPUT_CLUTCH 1
#define INPUT_SHIFT 2

#define MIN_WINNING_DISTANCE (97 * 256)

#define MAX_TACHOMETER 32
#define MAX_FRAME_COUNTER 16
#define MAX_GEAR 4
#define MAX_SPEED 256

#define MAX_STATES (MAX_TACHOMETER * MAX_SPEED * (MAX_GEAR + 1) * 2 * 2)



// todo: improve types (it does not need int for everything)
typedef struct GameState {
    int timer;
    int frame_counter;
    int tachometer;
    int tachometer_diff;
    int distance;
    int speed;
    int gear;
    int initial_tachometer;
    int initial_frame_counter;
    // todo: migrate to bitfield
    unsigned char inputs[MAX_FRAMES + 1];
} GameState;



/*
 * init_state will init a state from initial parameters.
 *
 * Based on OmniGamer's work, those initial parameters are tachometer and
 * frame_counter.
 */
void init_state(GameState* state, int tachometer, int frame_counter, int clutch, int shift) {
    bzero(state, sizeof(GameState));
    state->frame_counter = frame_counter;
    state->initial_frame_counter = frame_counter;
    state->tachometer = tachometer;
    state->initial_tachometer = tachometer;
    state->inputs[0] = (clutch * INPUT_CLUTCH) | (shift * INPUT_SHIFT);
    state->timer = 1;
    state->gear = INITIAL_GEAR;
    state->speed = INITIAL_SPEED;
};

/*
 * state_timer return the in-game timer for a specific frame.
 */
float state_timer(GameState state) {
    return trunc(state.timer * 3.34) / 100.0;
}

/*
 * hash_state return the hash associated to a state.
 */
int hash_state(GameState state) {
    return (
        (state.inputs[state.timer - 1] & INPUT_SHIFT ? 1 : 0)
        + 2 * state.gear
        + 2 * (MAX_GEAR + 1) * state.speed
        + 2 * (MAX_GEAR + 1) * MAX_SPEED * state.tachometer
        + 2 * (MAX_GEAR + 1) * MAX_SPEED * MAX_TACHOMETER * state.tachometer_diff
    );
}

/*
 * game_step run a single frame of the game Dragster.
 *
 * This function is currently based on OmniGamer's work, which is available on
 * Google Sheets:
 * https://docs.google.com/spreadsheets/d/1m1JKUGQdqjRkgqWgY6j6Dp1dXqM7KKEuYwjw7fpnLSM/edit
 *
 * Todo: make a full disassembly of the Atari 2600 ROM to improve and document
 * this function.
 */
void game_step(GameState* state, int clutch, int shift) {
    state->inputs[state->timer] = (clutch * INPUT_CLUTCH) | (shift * INPUT_SHIFT);
    state->timer++;
    state->frame_counter = (state->frame_counter + 2) % MAX_FRAME_COUNTER;

    // Update gear and tachometer.
    if (state->inputs[state->timer - 2] & INPUT_SHIFT) {
        state->gear = state->gear >= MAX_GEAR ? MAX_GEAR : state->gear + 1;

        if (clutch) {
            state->tachometer -= state->tachometer_diff - 3;
        } else {
            state->tachometer -= state->tachometer_diff + 3;
        }
    } else {
        if (!(state->frame_counter % (int) pow(2, state->gear))) {
            if (clutch) {
                state->tachometer -= state->tachometer_diff - 1;
            } else {
                state->tachometer -= state->tachometer_diff + 1;
            }
        } else {
            state->tachometer -= state->tachometer_diff;
        }
    }

    if (state->tachometer < 0) {
        state->tachometer = 0;
    }

    // Compute the speed limit.
    int speed_limit;
    if (state->tachometer >= 20 && state->gear > 1) {
        speed_limit = state->tachometer * pow(2, state->gear - 1) + pow(2, state->gear - 2);
    } else {
        speed_limit = state->tachometer * pow(2, state->gear - 1);
    }

    // Update tachometer difference, which post_tachometer - tachometer.
    if (state->inputs[state->timer - 2] & INPUT_SHIFT) {
        state->tachometer_diff = 0;
    } else {
        if (speed_limit - state->speed >= 16) {
            state->tachometer_diff = 1;
        } else {
            state->tachometer_diff = 0;
        }
    }

    // Update speed
    if (state->gear && !(state->inputs[state->timer - 2] & INPUT_SHIFT)) {
        if (state->speed > speed_limit) {
            state->speed -= 1;
        }
        if (state->speed < speed_limit) {
            state->speed += 2;
        }
    }

    // Update distance.
    state->distance += state->speed;
}

void debug_state(GameState* state, int mode) {
    GameState debug_state;
    memcpy(&debug_state, state, sizeof(GameState));
    debug_state.frame_counter = state->initial_frame_counter;
    debug_state.initial_frame_counter = state->initial_frame_counter;
    debug_state.tachometer = state->initial_tachometer;
    debug_state.initial_tachometer = state->initial_tachometer;
    debug_state.inputs[0] = state->inputs[0];
    debug_state.timer = 1;
    debug_state.distance = 0;
    debug_state.gear = INITIAL_GEAR;
    debug_state.speed = INITIAL_SPEED;

    int frame;
    int clutch;
    int shift;

    for (frame = 0; frame <= MAX_FRAMES; ++frame) {
        clutch = state->inputs[frame] & INPUT_CLUTCH ? 1 : 0;
        shift = state->inputs[frame] & INPUT_SHIFT ? 1 : 0;

        if (frame) {
            game_step(&debug_state, clutch, shift);
        }

        if (mode) {
            printf("%d: %d,%d | %d - %d - %d - %d - %d\n", frame, clutch, shift, debug_state.gear, debug_state.speed, debug_state.tachometer, debug_state.tachometer_diff, debug_state.distance);
        } else {
            printf("%d\t%d\n", shift, clutch);
        }
    }

    printf("Initial frame_counter: %d\n", debug_state.initial_frame_counter);
    printf("Initial tachometer: %d\n", debug_state.initial_tachometer);
}

int main() {
    int frame_counter, tachometer, clutch, shift;
    int index;

    GameState* states = malloc(MAX_STATES * sizeof(GameState));
    GameState* next_states = malloc(MAX_STATES * sizeof(GameState));

    GameState best_state = {0};
    best_state.timer = MAX_FRAMES;
    best_state.distance = 0;

    GameState* current_state;
    unsigned long long int total_simulations = 0;

    /*
     * There's no speed advantage to running all frame_counter configurations at
     * once, so we'll loop over the initial frame_counter since it's more
     * efficient memory-wise.
     */
    for (frame_counter = 0; frame_counter < MAX_FRAME_COUNTER; frame_counter += 2) {
        bzero(states, MAX_STATES * sizeof(GameState));
        bzero(next_states, MAX_STATES * sizeof(GameState));

        printf("Now testing all configurations with an initial frame counter equal to %d.\n", frame_counter);

        /*
         * Generating initial states, based on OmniGamer's model.
         */
        for (tachometer = 0; tachometer < MAX_TACHOMETER; tachometer += 3) {
            for (clutch = 0; clutch <= 1; ++clutch) {
                for (shift = 0; shift <= 1; ++shift) {
                    GameState initial_state;
                    init_state(&initial_state, tachometer, frame_counter, clutch, shift);
                    index = hash_state(initial_state);
                    memcpy(&(states[index]), &initial_state, sizeof(GameState));
                }
            }
        }


        int frame;
        GameState nextState;
        int next_index;
        int stop_configuration = 0;

        /*
         * This is the main loop: we generate all possible states from previous
         * generated ones, dropping those who won't be able to finish, and using
         * deduplication to greatly reduce the search space.
         */
        for (frame = 1; frame <= MAX_FRAMES && !stop_configuration; ++frame) {
            current_state = states;

            for (index = 0; index < MAX_STATES; ++index) {
                if (current_state->timer == frame) {
                    for (clutch = 0; clutch <= 1; ++clutch) {
                        for (shift = 0; shift <= 1; ++shift) {
                            memcpy(&nextState, current_state, sizeof(GameState));
                            game_step(&nextState, clutch, shift);
                            total_simulations++;

                            /*
                             * Dropping states which can't win anything.
                             *
                             * Todo: use bestState to detect which states won't
                             * be better than the best computed frame.
                             */
                            if ((nextState.tachometer < MAX_TACHOMETER) && (nextState.distance + MAX_SPEED * (MAX_FRAMES - frame) >= MIN_WINNING_DISTANCE)) {
                                if (nextState.distance >= MIN_WINNING_DISTANCE) {
                                    if ((nextState.timer < best_state.timer) || (nextState.timer == best_state.timer && nextState.distance > best_state.distance)) {
                                        memcpy(&best_state, &nextState, sizeof(GameState));
                                    }
                                    stop_configuration = 1;
                                }

                                next_index = hash_state(nextState);

                                /*
                                 * If a state collision occurs, it's safe to
                                 * keep the one which has the greatest distance.
                                 */
                                if (nextState.distance >= next_states[next_index].distance) {
                                    memcpy(&(next_states[next_index]), &nextState, sizeof(GameState));
                                }
                            }
                        }
                    }
                }
                ++current_state;
            }
            memcpy(states, next_states, MAX_STATES * sizeof(GameState));
            bzero(next_states, MAX_STATES * sizeof(GameState));
        }
    }

    free(states);
    free(next_states);

    printf("\n");

    if (0 == best_state.distance) {
        printf("It's not possible to do the race under %0.2fs.\n", state_timer(best_state));
        printf("%llu simulations were performed.\n", total_simulations);
        return EXIT_FAILURE;
    }

    printf("The best possible race is %0.2fs.\n", state_timer(best_state));
    printf("The best subdistance reachable with a %0.2fs timer is %d.\n", state_timer(best_state), best_state.distance % 256);
    printf("%llu simulations were performed.\n", total_simulations);

    // debug_state(&best_state, 1);
    // debug_state(&best_state, 0);

    return EXIT_SUCCESS;
}
