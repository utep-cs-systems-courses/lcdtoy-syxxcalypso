#ifndef GAME_H
#define GAME_H

#include <shape.h>

typedef struct transform_s {
  Layer *layer;
  Vec2 velocity;
  char (*CollisionCheck)(struct transform_s*, struct transform_s*);
  struct transform_s *next;
} transform_t;

static char HandleCollidePaddleLeft(struct transform_s *ball, struct transform_s *paddle);
static char HandleCollidePaddleRight(struct transform_s *ball, struct transform_s *paddle);

#endif // GAME_H
