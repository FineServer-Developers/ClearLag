﻿#pragma once
//#include<lbpch.h>
#define MCAPI __declspec(dllimport)
#define MCINLINE inline
#define MCCLS
#include <stl/Bstream.h>
#include <string>
#include "mass.h"
enum ActorType : int;
enum class AbilitiesIndex : int;
static inline int iround(float x) {
    int r = int(x);
    if (x < 0)
        r--;
    return r;
}

class BlockPos {
public:
    int x, y, z;
    inline bool operator==(BlockPos const& rv) const { return x == rv.x && y == rv.y && z == rv.z; }
    inline bool operator!=(BlockPos const& rv) const { return x != rv.x || y != rv.y || z != rv.z; }
    inline std::string toString() {
        return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    }
    template <typename _TP>
    void pack(WBStreamImpl<_TP>& ws) const {
        ws.apply(x, y, z);
    }
    void unpack(RBStream& rs) { rs.apply(x, y, z); }
    inline BlockPos add(int dx, int dy, int dz) {
        return { x + dx,y + dy,z + dz };
    }
};

class Vec3 {
  public:
    float x, y, z;
    std::string toString() {
        return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")";
    }
    inline BlockPos toBlockPos() {
        auto px = (int)x;
        auto py = (int)y;
        auto pz = (int)z;
        if (px < 0)px = px - 1;
        if (px < 0)py = py - 1;
        if (pz < 0)pz = pz - 1;
        return { px,py,pz };
    }
    template <typename _TP>
    void pack(WBStreamImpl<_TP> &ws) const {
        ws.apply(x, y, z);
    }
    void unpack(RBStream &rs) { rs.apply(x, y, z); }
    inline Vec3 add(float dx, float dy, float dz) {
        return { x + dx,y + dy,z + dz };
    }
};



struct IVec2 {
    int x, z;
    IVec2(Vec3 l) {
        x = iround(l.x);
        z = iround(l.z);
    }
    IVec2(int a, int b) : x(a), z(b) {}
    void operator+=(int v) {
        x += v;
        z += v;
    }
};
template <typename A, typename T>
class AutomaticID {
    T id;

  public:
    AutomaticID() { id = 0; }
    AutomaticID(T x) { id = x; }
    operator T() { return id; }
};
struct Tick {
    unsigned long long t;
};
struct ActorUniqueID {
    long long id;

  public:
    ActorUniqueID() { id = -1; }
    ActorUniqueID(long long i) { id = i; }
    auto get() { return id; }
    operator long long() { return id; }
};
static_assert(!std::is_pod_v<ActorUniqueID>);
class ActorRuntimeID {
  public:
    unsigned long long id;
    auto get() { return id; }
    operator unsigned long long() { return id; }
};
static_assert(std::is_pod_v<ActorRuntimeID>);
class ActorDamageSource {
  private:
    char filler[0x10];

  public:
    virtual void destruct1(unsigned int)     = 0;
    virtual bool isEntitySource() const      = 0;
    virtual bool isChildEntitySource() const = 0;

  private:
    virtual void *unk0() = 0;
    virtual void *unk1() = 0;  // death msg
    virtual void *unk2() = 0;  // is creative
    virtual void *unk3() = 0;  //?
  public:
    virtual ActorUniqueID getEntityUniqueID() const = 0;
    virtual int getEntityType() const               = 0;

  private:
    virtual int getEntityCategories() const = 0;
};

class ChunkPos {
  public:
    int x, z;
};

constexpr const int SAFE_PADDING = 0;