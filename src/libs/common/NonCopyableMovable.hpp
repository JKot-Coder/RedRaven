#pragma once

#define NONCOPYABLE(Type)       \
    Type(const Type&) = delete; \
    Type& operator=(const Type&) = delete;

#define NONMOVABLE(Type)   \
    Type(Type&&) = delete; \
    Type& operator=(Type&&) = delete;

#define NONCOPYABLE_MOVABLE(Type) \
    NONCOPYABLE(Type)             \
    NONMOVABLE(Type)