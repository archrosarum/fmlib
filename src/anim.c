#include "fmgui.h"
#include <math.h>

anim mkanim(vec_mes goal, double seconds)
{
    return (anim){.goal = goal, .seconds = seconds};
}

static bool valid_position(vec_mes pos)
{
    return pos.mode >= VEC_MES_INDEPENDENT && pos.mode <= VEC_MES_FROM_Y &&
        isfinite(pos.x.pct) && isfinite(pos.y.pct);
}

bool startanim(anim* animation, rect* rec)
{
    if (!animation || !rec || rec->is_winrect || !isfinite(animation->seconds) ||
        animation->seconds < 0 || !valid_position(animation->goal) ||
        !valid_position(rec->pos) || animation->goal.mode != rec->pos.mode) return false;
    animation->start = rec->pos;
    animation->elapsed = 0;
    animation->target = animation->seconds > 0 ? rec : NULL;
    if (animation->seconds == 0) rec->pos = animation->goal;
    return true;
}

static mes interpolate(mes start, mes goal, double t)
{
    /* Convert before subtracting to avoid integer overflow. Pixel offsets
     * truncate toward zero because mes stores whole pixels. */
    return (mes){
        (float)((double)start.pct + ((double)goal.pct - start.pct) * t),
        (int)((double)start.px + ((double)goal.px - start.px) * t)
    };
}

bool updanim(anim* animation, double delta_seconds)
{
    if (!animation || !animation->target) return false;
    if (!isfinite(delta_seconds) || delta_seconds <= 0) return true;
    if (delta_seconds >= animation->seconds - animation->elapsed) {
        animation->target->pos = animation->goal;
        animation->elapsed = animation->seconds;
        animation->target = NULL;
        return false;
    }
    animation->elapsed += delta_seconds;
    double t = animation->elapsed / animation->seconds;
    animation->target->pos = (vec_mes){
        .x = interpolate(animation->start.x, animation->goal.x, t),
        .y = interpolate(animation->start.y, animation->goal.y, t),
        .mode = animation->goal.mode
    };
    return true;
}
