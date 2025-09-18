#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <raylib.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef long double f128;

#define CELLSIZE 40 
#define WIDTH 10
#define HEIGHT 10
#define FRAMERATE 60
#define UPDATE_PER_SEC 4.0 

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define LOG(fmt, ...) \
	fprintf(stderr, fmt, __VA_ARGS__); \
	fflush(stderr);

typedef enum State {
	PLAY,
	PAUSE,
	WIN,
	LOSE,
} State;

typedef enum Dir : u8 {
	UP,
	LEFT,
	DOWN,
	RIGHT,
} Dir;

char *dir_to_str(Dir dir) {
	switch (dir) {
	case UP:
		return "UP";
	case LEFT:
		return "LEFT";
	case DOWN:
		return "DOWN";
	case RIGHT:
		return "RIGHT";
	}
}

typedef struct Loc {
	i32 x;
	i32 y;
} Loc;

Loc loc_rand() {
	return (Loc){
		.x = (i32)floor(WIDTH * ((f64)rand() / RAND_MAX)),
		.y = (i32)floor(HEIGHT * ((f64)rand() / RAND_MAX)),
	};
}

bool loc_eq(Loc a, Loc b) {
	return a.x == b.x && a.y == b.y;
}

bool loc_contained(Loc x, Loc *list, size_t listlen) {
	for (size_t i = 0; i < listlen - 1; i++) {
		if (loc_eq(x, list[i])) {
			return true;
		}
	}

	return false;
}

Loc loc_rand_until_diff(Loc *complist, size_t listlen, u32 maxattempts) {
	for(size_t i = 0; i < maxattempts; i++) {
		Loc loc = loc_rand();
		if (loc_contained(loc, complist, listlen)) {
			continue;
		}

		return loc;
	}

	return (Loc){.x = 0, .y = 0};
}

void update_loc(Loc *loc, Dir dir) {
	switch (dir) {
	case UP:
		loc->y = MAX(loc->y - 1, 0);
		break;
	case RIGHT:
		loc->x = MIN(loc->x + 1, WIDTH - 1);
		break;
	case DOWN:
		loc->y = MIN(loc->y + 1, HEIGHT - 1);
		break;
	case LEFT:
		loc->x = MAX(loc->x - 1, 0);
		break;
	}
}

void draw_text_center(char *text) {
	i32 fontsize = CELLSIZE;
	int lensize = MeasureText(text, fontsize);

	DrawText(
		text,
		// it can be simplified with math, probably 
		WIDTH * CELLSIZE / 2 - lensize / 2,
		HEIGHT * CELLSIZE / 2 - fontsize / 2,
		fontsize,
		WHITE
	);
}

int main() {
	srand((u32)time(nullptr));

	InitWindow(CELLSIZE * WIDTH, CELLSIZE * HEIGHT, "Snake");

	SetTargetFPS(FRAMERATE);

	State state = PLAY;

	size_t slenmax = WIDTH * HEIGHT;
	Loc slocs[slenmax];
	Dir sdirs[slenmax];
	size_t slen = 1;

	slocs[0] = loc_rand();
	sdirs[0] = RIGHT;

	u32 redzonex = WIDTH / 3;
	u32 redzoney = HEIGHT / 3;
	if (slocs[0].x < redzonex) {
		sdirs[0] = RIGHT;
	} else if (slocs[0].x >= WIDTH - redzonex) {
		sdirs[0] = LEFT;
	} else if (slocs[0].y < redzoney) {
		sdirs[0] = DOWN;
	} else if (slocs[0].y >= HEIGHT - redzoney) {
		sdirs[0] = UP;
	}

	Loc foodloc = loc_rand_until_diff(slocs, slen, WIDTH * HEIGHT * 2);

	Dir dirnext = sdirs[0];
	bool eaten = false;

	u32 updaterate = (u32)(FRAMERATE / UPDATE_PER_SEC);
	u32 framecount = 0;
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_SPACE)) {
			if (state == PLAY) {
				state = PAUSE;
			} else if (state == PAUSE) {
				state = PLAY;
			}
		}

		if (IsKeyPressed(KEY_UP)) dirnext = UP;
		if (IsKeyPressed(KEY_W)) dirnext = UP;
		if (IsKeyPressed(KEY_RIGHT)) dirnext = RIGHT;
		if (IsKeyPressed(KEY_D)) dirnext = RIGHT;
		if (IsKeyPressed(KEY_DOWN)) dirnext = DOWN;
		if (IsKeyPressed(KEY_S)) dirnext = DOWN;
		if (IsKeyPressed(KEY_LEFT)) dirnext = LEFT;
		if (IsKeyPressed(KEY_A)) dirnext = LEFT;

		if (state == PLAY && ++framecount >= updaterate) {
			framecount = 0;

			if (
				(sdirs[0] == UP && dirnext != DOWN) ||
				(sdirs[0] == LEFT && dirnext != RIGHT) ||
				(sdirs[0] == DOWN && dirnext != UP) ||
				(sdirs[0] == RIGHT && dirnext != LEFT)
		    ) {
				sdirs[0] = dirnext;
			}

			if (!eaten) {
				Dir dirswap = sdirs[0];
				for (size_t i = 0; i < slen; i++) {
					update_loc(&slocs[i], sdirs[i]);

					if (i > 0) {
						if (loc_eq(slocs[0], slocs[i])) {
							state = LOSE;
						}

						Dir dirtemp = sdirs[i];
						sdirs[i] = dirswap;
						dirswap = dirtemp;
					}
				}
			} else {
				eaten = false;

				slen++;
				for (size_t k = 0; k < slen - 1; k++) {
					size_t i = slen - 1 - k;

					slocs[i] = slocs[i - 1];
					sdirs[i] = sdirs[i - 1];
				}

				update_loc(&slocs[0], sdirs[0]);

				if (slen == slenmax) {
					state = WIN;
				}
			}

			if (state == PLAY && loc_eq(slocs[0], foodloc)) {
				eaten = true;

				foodloc = loc_rand_until_diff(slocs, slen, WIDTH * HEIGHT * 2);
			}
		}

		BeginDrawing();

		{
			ClearBackground(DARKGRAY);

			DrawRectangle(
				foodloc.x * CELLSIZE,
				foodloc.y * CELLSIZE,
				CELLSIZE,
				CELLSIZE,
				BLUE
			);

			for (size_t i = 0; i < slen; i++) {
				DrawRectangle(
					slocs[i].x * CELLSIZE,
					slocs[i].y * CELLSIZE,
					CELLSIZE,
					CELLSIZE,
					i == 0 ? RED : YELLOW
				);
			}

			if (state > PLAY) {
				DrawRectangle(
					0,
					0,
					WIDTH * CELLSIZE,
					HEIGHT * CELLSIZE,
					(Color){ .r = 0, .g = 0, .b = 0, .a = 125 }
				);

				switch (state) {
					case PLAY:
						// should never happen
						break;
					case PAUSE:
						draw_text_center("Pause");
						break;
					case WIN:
						draw_text_center("Victory");
						break;
					case LOSE:
						draw_text_center("Game Over");
						break;
				}
			}
		}

		EndDrawing();
	}

	CloseWindow();

	exit(EXIT_SUCCESS);
}
