#ifndef ENTITY_H
#define ENTITY_H

// originator of another entity
// handle to originator, entity cant collide with its originator
// collision layers (1,2,3,4,5)
typedef enum EntityFlag {
    EntityFlag_Active =        (1 << 0),
    EntityFlag_MoveWithCtrls = (1 << 1),
    EntityFlag_CanCollide =    (1 << 2),
    EntityFlag_CanShoot =      (1 << 3),
    EntityFlag_MoveWithPhys =  (1 << 4),
    EntityFlag_IsProjectile =  (1 << 5),
    EntityFlag_Wrapping =      (1 << 6),
    EntityFlag_Particle =      (1 << 7),
} EntityFlag;

typedef enum CollisionType {
    CollisionType_None,
    CollisionType_Explode,
    CollisionType_Health,
    CollisionType_SplinterOnDeath,
    CollisionType_HealthOrSplinter,
    CollisionType_Count,
} CollisionType;

typedef enum DeathType{
    DeathType_Crumble,
    DeathType_Particle,
    DeathType_Animate
} DeathType;

typedef enum ParticleType{
    ParticleType_Exhaust,
    ParticleType_Bullet,
} ParticleType;

typedef enum EntityType {EntityType_None, EntityType_Quad, EntityType_Texture, EntityType_Text, EntityType_Line, EntityType_Structure, EntityType_Skeleton1} EntityType;

typedef enum StructureType{
    StructureType_None,
    StructureType_Castle,
    StructureType_Count,
} StructureType;

typedef enum EntityCommandType{
    EntityCommandType_None,
    EntityCommandType_Move,
    EntityCommandType_Count,
} EntityCommandType;

typedef struct EntityCommand{
    EntityCommandType type;
    v2 move_to;
    v2 clicked_at;
} EntityCommand;

#define ENTITY_COMMANDS_MAX 1024
typedef struct Entity{
    Entity* origin;
    Entity* parent;
    Entity* next;
    Entity* prev;

    EntityType type;
    DeathType death_type;
    ParticleType particle_type;

    // structure info
    StructureType structure_type;
    v2 rallypoint;
    v2 rallypoint_cell;

    u32 flags;
    u32 collision_type;
    u32 index;
    u32 generation;

    union{
        struct{
            f32 x;
            f32 y;
        };
        v2 pos;
    };
    v2 cell;
    v2 move_to_target;

    v2 dim;
    v2 rot;
    f32 deg;

    v2 dir;
    v2 accel_dir;
    f32 accel_deg;

    RGBA color;

    f32 collision_padding;

    f32 speed;
    f32 rotation_speed;
    f32 accel_speed;
    f32 shoot_t;
    v2 velocity;

    EntityCommand commands[ENTITY_COMMANDS_MAX];
    EntityCommand* active_command;
    u32 commands_read;
    u32 commands_write;

    //s32 health;
    //s32 damage;
    //bool in_play;
    //f32 particle_t;
    //bool accelerating;
    //bool exploding;
    //u32  explosion_tex;
    //f32 explosion_t;
    //f32 immune_t;
    //bool immune;

    //bool dead;
    bool selected;
    TextureAsset texture;
} Entity;

static bool has_flag(u32 lflags, u32 rflags);
static void set_flag(u32* lflags, u32 rflags);
static void clear_flag(u32* lflags, u32 rflags);

typedef struct EntityHandle{
    u32 index;
    u32 generation;
} EntityHandle;

static EntityHandle zero_entity_handle(void);
//static Entity* entity_from_handle(PermanentMemory* pm, EntityHandle handle);
//static EntityHandle handle_from_entity(PermanentMemory* pm, Entity *e);
static Rect rect_from_entity(Entity* e);
static Rect collision_box_from_entity(Entity* e);
static Quad bounding_box_from_entity(Entity* e);

static u32 entity_commands_count(Entity* e);
static bool entity_commands_empty(Entity* e);
static bool entity_commands_full(Entity* e);
static void entity_commands_clear(Entity* e);

static void entity_commands_add(Entity* e, EntityCommand c);
static EntityCommand* entity_commands_next(Entity* e);

static void entity_commands_move(Entity* e, v2 move_to, v2 clicked_at);

#endif
