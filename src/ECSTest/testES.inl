#include "components.hpp"

void updateES(const UpdateEvent& evnt, Position& pos, Velocity vel)
{
    pos.x += vel.x * evnt.dt;
    pos.y += vel.y * evnt.dt;
}