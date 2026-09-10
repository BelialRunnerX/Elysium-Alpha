#include "game/Game.hpp"

#include "core/Determinism.hpp"
#include "world/Block.hpp"
#include "world/SurfaceChunkPersistence.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace elysium {
namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr const char* kSavePath = "elysium_save.txt";
constexpr const char* kSurfaceChunkSaveRoot = "elysium_save_chunks";

Vector3 rv(Vec3 v) { return {v.x, v.y, v.z}; }
Color rc(Color4u c) { return {c.r,c.g,c.b,c.a}; }

float distance2D(Vec3 a, Vec3 b) {
    const float dx = a.x - b.x;
    const float dz = a.z - b.z;
    return std::sqrt(dx*dx + dz*dz);
}

void drawBar(int x, int y, int w, int h, float value, const char* label) {
    DrawRectangle(x, y, w, h, Color{20,24,28,220});
    DrawRectangle(x + 2, y + 2, static_cast<int>((w - 4) * std::clamp(value,0.0f,100.0f) / 100.0f), h - 4, Color{178,196,185,255});
    DrawRectangleLines(x, y, w, h, Color{225,230,226,180});
    DrawText(TextFormat("%s %3.0f", label, value), x + 6, y + 3, 14, RAYWHITE);
}

const char* planetName(PlanetClass p) {
    switch (p) {
        case PlanetClass::Temperate: return "TEMPERATE // FRONTIER PLAINS";
        case PlanetClass::Barren: return "BARREN // REGOLITH PLAIN";
        case PlanetClass::Scorched: return "SCORCHED // BASALT PLAIN";
        case PlanetClass::Frozen: return "FROZEN // CRYOVOLCANIC FRONTIER";
        case PlanetClass::Toxic: return "TOXIC // CAUSTIC BIOSPHERE";
        case PlanetClass::Irradiated: return "IRRADIATED // RAD-STORM WASTE";
        case PlanetClass::Oceanic: return "OCEANIC // PELAGIC WORLD";
        case PlanetClass::Anomalous: return "ANOMALOUS // IMPERIAL EXCLUSION ZONE";
    }
    return "UNKNOWN";
}

EnemyArchetype enemyArchetypeFor(ImperialUnitRole role) {
    switch(role) {
        case ImperialUnitRole::Drone: return EnemyArchetype::Drone;
        case ImperialUnitRole::Lictor: return EnemyArchetype::Lictor;
        case ImperialUnitRole::Adept: return EnemyArchetype::Adept;
        case ImperialUnitRole::Praetor: return EnemyArchetype::Praetor;
    }
    return EnemyArchetype::Drone;
}

float enemyRenderRadius(EnemyArchetype archetype) {
    switch(archetype) {
        case EnemyArchetype::Drone: return 0.42f;
        case EnemyArchetype::Lictor: return 0.58f;
        case EnemyArchetype::Adept: return 0.46f;
        case EnemyArchetype::Praetor: return 0.82f;
    }
    return 0.42f;
}

Color enemyRenderColor(EnemyArchetype archetype) {
    switch(archetype) {
        case EnemyArchetype::Drone: return Color{28,34,31,255};
        case EnemyArchetype::Lictor: return Color{48,55,51,255};
        case EnemyArchetype::Adept: return Color{28,42,46,255};
        case EnemyArchetype::Praetor: return Color{20,24,23,255};
    }
    return Color{28,34,31,255};
}

Color enemyAccentColor(EnemyArchetype archetype) {
    switch(archetype) {
        case EnemyArchetype::Drone: return Color{49,215,142,255};
        case EnemyArchetype::Lictor: return Color{129,224,172,255};
        case EnemyArchetype::Adept: return Color{90,189,238,255};
        case EnemyArchetype::Praetor: return Color{74,255,166,255};
    }
    return Color{49,215,142,255};
}

void drawSurfaceCellWires(const PlanetSurface& planet, SurfaceCellAddress a, Color color) {
    a=planet.normalize(a);
    const Vec3 a000=planet.boundaryPosition(a.face,a.u,a.v,a.radial);
    const Vec3 a100=planet.boundaryPosition(a.face,a.u+1,a.v,a.radial);
    const Vec3 a110=planet.boundaryPosition(a.face,a.u+1,a.v+1,a.radial);
    const Vec3 a010=planet.boundaryPosition(a.face,a.u,a.v+1,a.radial);
    const Vec3 a001=planet.boundaryPosition(a.face,a.u,a.v,a.radial+1);
    const Vec3 a101=planet.boundaryPosition(a.face,a.u+1,a.v,a.radial+1);
    const Vec3 a111=planet.boundaryPosition(a.face,a.u+1,a.v+1,a.radial+1);
    const Vec3 a011=planet.boundaryPosition(a.face,a.u,a.v+1,a.radial+1);
    DrawLine3D(rv(a000),rv(a100),color); DrawLine3D(rv(a100),rv(a110),color);
    DrawLine3D(rv(a110),rv(a010),color); DrawLine3D(rv(a010),rv(a000),color);
    DrawLine3D(rv(a001),rv(a101),color); DrawLine3D(rv(a101),rv(a111),color);
    DrawLine3D(rv(a111),rv(a011),color); DrawLine3D(rv(a011),rv(a001),color);
    DrawLine3D(rv(a000),rv(a001),color); DrawLine3D(rv(a100),rv(a101),color);
    DrawLine3D(rv(a110),rv(a111),color); DrawLine3D(rv(a010),rv(a011),color);
}

} // namespace

Game::Game() : renderer_(jobs_, graphics_), planetRenderer_(jobs_, graphics_) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1600, 960, "ELYSIUM // Open World Alpha");
    SetTargetFPS(60);
    DisableCursor();

    camera_.position = {0,0,0};
    camera_.target = {0,0,1};
    camera_.up = {0,1,0};
    camera_.fovy = 72.0f;
    camera_.projection = CAMERA_PERSPECTIVE;

    initWorlds();
    ecs_.createPlayer(spawnPoint(world()));

    // Survival starter kit. Resources remain scarce enough to make mining useful.
    ecs_.addItem(static_cast<int>(BlockType::Dirt), 20);
    ecs_.addItem(static_cast<int>(BlockType::Planks), 24);
    ecs_.addItem(static_cast<int>(BlockType::SteelPlate), 4);
    ecs_.addItem(static_cast<int>(BlockType::RegistryBeacon), 1);
    // Vertical-slice infrastructure test kit. Production progression will move
    // these behind the intended machine recipes/research gates.
    ecs_.addItem(machineItemId(MachineType::BurnerGenerator), 1);
    ecs_.addItem(machineItemId(MachineType::BatteryBank), 1);
    ecs_.addItem(machineItemId(MachineType::AtmosphereUnit), 1);
    ecs_.addItem(machineItemId(MachineType::StorageCrate), 1);
    ecs_.addItem(machineItemId(MachineType::AirlockController), 1);
    ecs_.addItem(machineItemId(MachineType::SensorMast), 1);
    ecs_.addItem(machineItemId(MachineType::Turret), 1);
    ecs_.addItem(machineItemId(MachineType::ShieldPylon), 1);
    ecs_.addItem(machineItemId(MachineType::LogicController), 1);
    ecs_.addItem(machineItemId(MachineType::Furnace), 1);
    ecs_.addItem(machineItemId(MachineType::AlloyCrucible), 1);
    ecs_.addItem(machineItemId(MachineType::Refinery), 1);
    ecs_.addItem(machineItemId(MachineType::NetworkStorage), 1);
    ecs_.addItem(machineItemId(MachineType::Conveyor), 2);
    ecs_.addItem(machineItemId(MachineType::Sorter), 1);
    ecs_.addItem(machineItemId(MachineType::CargoLoader), 1);
    ecs_.addItem(machineItemId(MachineType::Crusher), 1);
    ecs_.addItem(machineItemId(MachineType::ChemicalVat), 1);
    ecs_.addItem(machineItemId(MachineType::Fabricator), 1);
    ecs_.addItem(machineItemId(MachineType::Extractor), 1);
    ecs_.addItem(surfacePortalItemId(SurfacePortalType::Door), 2);
    ecs_.addItem(surfacePortalItemId(SurfacePortalType::Airlock), 2);

    titleScreen_ = true;
    cinematicHud_ = true;
    if (!loadGame()) setMessage("UNFILED. Spherical surface online. Mine, build, survive.", 5.0f);
}

Game::~Game() {
    saveGame();
    renderer_.invalidate();
    planetRenderer_.invalidate();
    CloseWindow();
}

void Game::initWorlds() {
    const std::array<PlanetClass,PlanetCount> classes{
        PlanetClass::Temperate, PlanetClass::Barren, PlanetClass::Scorched, PlanetClass::Frozen,
        PlanetClass::Toxic, PlanetClass::Irradiated, PlanetClass::Oceanic, PlanetClass::Anomalous
    };
    const std::array<std::uint64_t,PlanetCount> labels{
        0x54454D50ULL,0x42415252ULL,0x53434F52ULL,0x46524F5AULL,
        0x544F5843ULL,0x49525244ULL,0x4F434541ULL,0x414E4F4DULL
    };
    for (int i=0;i<PlanetCount;++i) {
        const std::uint64_t seed=mix64(galaxySeed_^labels[static_cast<std::size_t>(i)]);
        worlds_[static_cast<std::size_t>(i)] = std::make_unique<World>(seed,classes[static_cast<std::size_t>(i)]);
        planetSurfaces_[static_cast<std::size_t>(i)] = std::make_unique<PlanetSurface>(seed,classes[static_cast<std::size_t>(i)]);
        bases_[static_cast<std::size_t>(i)] = std::make_unique<BaseInfrastructure>(seed);
        surfaceBases_[static_cast<std::size_t>(i)] = std::make_unique<SurfaceInfrastructure>(seed);
        surfaceNavigation_[static_cast<std::size_t>(i)] = std::make_unique<SurfaceNavigationService>(*planetSurfaces_[static_cast<std::size_t>(i)],64,768);
        surfaceSieges_[static_cast<std::size_t>(i)] = std::make_unique<SurfaceSiegeDirector>(seed);

        // Cube-sphere player position is stable per planet. v0.6 also migrates
        // MicroBricks and base-machine anchors into this same address domain;
        // the legacy planar patch remains only as a compatibility sandbox.
        const int u=PlanetSurface::FaceResolution/2;
        const int v=PlanetSurface::FaceResolution/2;
        const Vec3 dir=faceGridCellDirection(CubeFace::PositiveZ,u,v,PlanetSurface::FaceResolution);
        const float ground=planetSurfaces_[static_cast<std::size_t>(i)]->surfaceBoundaryRadius(dir);
        sphericalPlayerPositions_[static_cast<std::size_t>(i)] = dir*(ground+0.05f);
    }
}

Vec3 Game::spawnPoint(const World& w) const {
    constexpr int x = World::Width / 2;
    constexpr int z = World::Depth / 2;
    return {x + 0.5f, static_cast<float>(w.surfaceY(x,z)) + 1.01f, z + 0.5f};
}

Vec3 Game::shipTerminalPosition(const World& w) const {
    constexpr int x = World::Width / 2 + 5;
    constexpr int z = World::Depth / 2;
    return {x + 0.5f, static_cast<float>(w.surfaceY(x,z)) + 1.0f, z + 0.5f};
}

Vec3 Game::sphericalShipTerminalPosition() const {
    const int u=PlanetSurface::FaceResolution/2 + 5;
    const int v=PlanetSurface::FaceResolution/2;
    const Vec3 dir=faceGridCellDirection(CubeFace::PositiveZ,u,v,PlanetSurface::FaceResolution);
    return dir*(surfaceRead().surfaceBoundaryRadius(dir)+0.80f);
}

Vec3 Game::sphericalArrivalPosition() const {
    const int u=PlanetSurface::FaceResolution/2 + 2;
    const int v=PlanetSurface::FaceResolution/2;
    const Vec3 dir=faceGridCellDirection(CubeFace::PositiveZ,u,v,PlanetSurface::FaceResolution);
    return dir*(surfaceRead().surfaceBoundaryRadius(dir)+0.05f);
}

Vec3 Game::cameraPosition() const {
    return ecs_.player().position + Vec3{0.0f, 1.62f, 0.0f};
}

Vec3 Game::cameraForward() const {
    const float cp = std::cos(pitch_);
    return normalize(Vec3{std::sin(yaw_) * cp, std::sin(pitch_), std::cos(yaw_) * cp});
}

Vec3 Game::sphericalCameraPosition() const {
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const Vec3 up=normalize(feet);
    return feet + up*1.62f;
}

Vec3 Game::sphericalCameraForward() const {
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const auto frame=planetSurface().surfaceFrame(feet);
    const Vec3 horizontal=normalize(frame.forward*std::cos(yaw_)+frame.right*std::sin(yaw_));
    return normalize(horizontal*std::cos(pitch_)+frame.up*std::sin(pitch_));
}

void Game::run() {
    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.05f);
        worldTimeOfDay_ = std::fmod(worldTimeOfDay_ + dt * 0.008f, 1.0f);
        if (titleScreen_) {
            titleTimer_ += dt;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                titleScreen_ = false;
                setMessage("DESCENT COMPLETE // cube-sphere surface online", 3.5f);
            }
            drawTitleScreen();
            continue;
        }
        if (IsKeyPressed(KEY_F3)) cinematicHud_ = !cinematicHud_;
        update(dt);
        draw();
    }
}

void Game::update(float dt) {
    if (messageTimer_ > 0.0f) messageTimer_ -= dt;
    surfaceAiTelemetry_={};
    surfaceDefenseTelemetry_={};
    surfaceIndustryTelemetry_={};

    if (IsKeyPressed(KEY_F5)) saveGame();
    if (IsKeyPressed(KEY_F9)) {
        if (loadGame()) setMessage("Save reloaded.");
        else setMessage("No valid save found.");
    }

    if (IsKeyPressed(KEY_P)) {
        sphericalSurfaceMode_ = !sphericalSurfaceMode_;
        orbitalPreview_ = false;
        travelMenu_ = craftingMenu_ = mapMenu_ = false;
        sphericalVerticalVelocity_=0.0f;
        sphericalGrounded_=true;
        miningProgress_=0.0f;
        sphericalMiningValid_=false;
        ecs_.clearEnemies();
        setMessage(sphericalSurfaceMode_ ? "PLANET SURFACE // streamed cube-sphere traversal + combat online" : "LEGACY PATCH // local systems view",2.8f);
    }
    if (IsKeyPressed(KEY_O)) {
        orbitalPreview_ = !orbitalPreview_;
        sphericalSurfaceMode_ = false;
        travelMenu_ = craftingMenu_ = mapMenu_ = false;
        if (orbitalPreview_ && lengthSq(orbitWalkerPosition_)<1.0f)
            orbitWalkerPosition_=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]+normalize(sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)])*1.2f;
        ecs_.clearEnemies();
        setMessage(orbitalPreview_ ? "ORBITAL FIELD LOD // topology/debug view" : "LEGACY PATCH // local systems view",2.5f);
    }

    if (orbitalPreview_) {
        updateOrbitalPreview(dt);
    } else if (sphericalSurfaceMode_) {
        updateMenus();
        const Vec3 streamFocus=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
        planetRenderer_.setStreamingFocus(streamFocus,12,16);
        planetRenderer_.sync(planetSurface());
        surfaceBase().update(planetSurface(),dt);
        surfaceIndustry_.update(surfaceBase(),dt);
        surfaceIndustry_.updateExtraction(planetSurface(),surfaceBase(),dt);
        surfaceIndustryTelemetry_=surfaceIndustry_.telemetry();
        if(surfaceIndustryTelemetry_.extractorSuspicionGenerated>0.0f)
            raiseSuspicion(surfaceIndustryTelemetry_.extractorSuspicionGenerated);
        if (!travelMenu_ && !craftingMenu_ && !mapMenu_) {
            selectHotbar();
            updateSphericalPlayer(dt);
            updateSphericalInteraction(dt);
            const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
            const Vec3 playerCenter=feet+normalize(feet)*0.9f;
            SurfaceWorldReadService read(planetSurface(),planetRenderer_.cpuChunkCache());
            auto hover=[this,&read](Vec3 candidate){
                Vec3 dir=lengthSq(candidate)>0.001f?normalize(candidate):Vec3{0,0,1};
                return dir*(read.surfaceBoundaryRadius(dir)+1.8f);
            };
            auto& navigation=surfaceNavigation();
            auto path=[&read,&navigation,playerCenter](Vec3 position,Vec3 velocity,float pathDt) {
                return navigation.advance(read,position,playerCenter,length(velocity),pathDt,1.8f,0.38f,1.6f);
            };
            auto mitigate=[this,playerCenter](Vec3 /*attacker*/,float damage) {
                return surfaceBase().absorbShieldDamage(planetSurface(),playerCenter,damage);
            };
            surfaceAiTelemetry_=ecs_.updateSurfaceEnemiesPhased(jobs_,dt,playerCenter,hover,path,mitigate);

            std::vector<SurfaceHostileContact> contacts;
            for(const auto& enemy:ecs_.enemies(ActorDomain::PlanetSurface))
                contacts.push_back({enemy.stableId,enemy.position,enemy.health});
            surfaceDefenseTelemetry_=surfaceBase().updateDefense(planetSurface(),dt,contacts);
            for(const auto& fire:surfaceBase().consumeTurretFireRequests())
                ecs_.queueDamageEnemy(fire.targetStableId,fire.damage);
            ecs_.flushCommands();
            updateEmpire(dt);
        }
    } else {
        updateMenus();
        if (!travelMenu_ && !craftingMenu_ && !mapMenu_) {
            base().update(dt);
            selectHotbar();
            updatePlayer(dt);
            updateInteraction(dt);
            updateEmpire(dt);
        }
    }

    const float floor = claimFloor();
    if (suspicion_ > floor) suspicion_ = std::max(floor, suspicion_ - 0.06f * dt);

    auto p = ecs_.player();
    const bool planarOutOfWorld=!sphericalSurfaceMode_ && !orbitalPreview_ && p.position.y < -10.0f;
    if (p.health <= 0.0f || planarOutOfWorld) {
        PlayerPersistentState state = ecs_.savePlayer();
        state.position = spawnPoint(world());
        state.health = 100.0f;
        state.oxygen = 100.0f;
        state.energy = 100.0f;
        state.hunger = 100.0f;
        ecs_.loadPlayer(state);
        const Vec3 dir=faceGridCellDirection(CubeFace::PositiveZ,PlanetSurface::FaceResolution/2,PlanetSurface::FaceResolution/2,PlanetSurface::FaceResolution);
        sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]=dir*(surfaceRead().surfaceBoundaryRadius(dir)+0.05f);
        sphericalVerticalVelocity_=0.0f;
        sphericalGrounded_=true;
        sphericalSurfaceMode_=true;
        orbitalPreview_=false;
        sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]=sphericalArrivalPosition();
        setMessage("FIELD FAILURE RECORDED // recovered at spherical ship", 4.0f);
    }
}

void Game::updateMenus() {
    if (IsKeyPressed(KEY_T) && nearShip()) {
        travelMenu_ = !travelMenu_;
        craftingMenu_ = false;
        mapMenu_ = false;
    }
    if (IsKeyPressed(KEY_C)) {
        craftingMenu_ = !craftingMenu_;
        travelMenu_ = false;
        mapMenu_ = false;
    }
    if (IsKeyPressed(KEY_M)) {
        mapMenu_ = !mapMenu_;
        travelMenu_ = false;
        craftingMenu_ = false;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        travelMenu_ = craftingMenu_ = mapMenu_ = false;
    }

    if (travelMenu_) {
        if (IsKeyPressed(KEY_ONE)) switchPlanet(0);
        if (IsKeyPressed(KEY_TWO)) switchPlanet(1);
        if (IsKeyPressed(KEY_THREE)) switchPlanet(2);
        if (IsKeyPressed(KEY_FOUR)) switchPlanet(3);
        if (IsKeyPressed(KEY_FIVE)) switchPlanet(4);
        if (IsKeyPressed(KEY_SIX)) switchPlanet(5);
        if (IsKeyPressed(KEY_SEVEN)) switchPlanet(6);
        if (IsKeyPressed(KEY_EIGHT)) switchPlanet(7);
    }
    if (craftingMenu_) {
        if (IsKeyPressed(KEY_ONE)) tryCraft(1);
        if (IsKeyPressed(KEY_TWO)) tryCraft(2);
        if (IsKeyPressed(KEY_THREE)) tryCraft(3);
        if (IsKeyPressed(KEY_FOUR)) tryCraft(4);
        if (IsKeyPressed(KEY_FIVE)) tryCraft(5);
        if (IsKeyPressed(KEY_SIX)) tryCraft(6);
        if (IsKeyPressed(KEY_SEVEN)) tryCraft(7);
        if (IsKeyPressed(KEY_EIGHT)) tryCraft(8);
        if (IsKeyPressed(KEY_NINE)) tryCraft(9);
        if (IsKeyPressed(KEY_ZERO)) tryCraft(10);
        if (IsKeyPressed(KEY_K)) tryCraft(11);
        if (IsKeyPressed(KEY_J)) tryCraft(12);
        if (IsKeyPressed(KEY_U)) tryCraft(13);
        if (IsKeyPressed(KEY_Y)) tryCraft(14);
        if (IsKeyPressed(KEY_L)) tryCraft(15);
        if (IsKeyPressed(KEY_G)) tryCraft(16);
        if (IsKeyPressed(KEY_H)) tryCraft(17);
        if (IsKeyPressed(KEY_I)) tryCraft(18);
        if (IsKeyPressed(KEY_N)) tryCraft(19);
        if (IsKeyPressed(KEY_Q)) tryCraft(20);
        if (IsKeyPressed(KEY_R)) tryCraft(21);
        if (IsKeyPressed(KEY_X)) tryCraft(22);
        if (IsKeyPressed(KEY_Z)) tryCraft(23);
        if (IsKeyPressed(KEY_F1)) tryCraft(24);
        if (IsKeyPressed(KEY_F2)) tryCraft(25);
        if (IsKeyPressed(KEY_F3)) tryCraft(26);
    }
}

void Game::updateOrbitalPreview(float dt) {
    const Vector2 mouse=GetMouseDelta();
    orbitYaw_ += mouse.x*0.0030f;
    orbitPitch_ -= mouse.y*0.0030f;
    orbitPitch_=std::clamp(orbitPitch_,-1.35f,1.35f);
    orbitDistance_=std::clamp(orbitDistance_-GetMouseWheelMove()*4.0f,58.0f,180.0f);

    // A live topology probe walks the curved substrate using a local tangent
    // frame. Direction->face ownership is recomputed after every move, so this
    // visibly crosses cube seams without teleporting between coordinate charts.
    if (lengthSq(orbitWalkerPosition_)>1.0f) {
        const auto frame=planetSurface().surfaceFrame(orbitWalkerPosition_);
        float forward=(IsKeyDown(KEY_W)?1.0f:0.0f)-(IsKeyDown(KEY_S)?1.0f:0.0f);
        float right=(IsKeyDown(KEY_D)?1.0f:0.0f)-(IsKeyDown(KEY_A)?1.0f:0.0f);
        Vec3 wish=frame.forward*forward+frame.right*right;
        if (lengthSq(wish)>1.0f) wish=normalize(wish);
        if (lengthSq(wish)>0.001f) {
            const float radius=std::max(1.0f,length(orbitWalkerPosition_));
            const Vec3 dir=normalize(normalize(orbitWalkerPosition_)+wish*(8.0f*dt/radius));
            const auto cell=planetSurface().locate(dir*planetSurface().referenceRadius());
            const int surface=planetSurface().surfaceRadial(cell.face,cell.u,cell.v);
            const float targetRadius=planetSurface().referenceRadius()+static_cast<float>(surface+1-PlanetSurface::ReferenceRadial)+1.2f;
            orbitWalkerPosition_=dir*targetRadius;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) orbitalPreview_=false;
}

void Game::updateSphericalPlayer(float dt) {
    const Vector2 mouse=GetMouseDelta();
    yaw_ += mouse.x*0.00225f;
    pitch_ -= mouse.y*0.00225f;
    pitch_=std::clamp(pitch_,-1.48f,1.48f);

    Vec3& feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    if (lengthSq(feet)<1.0f) {
        const Vec3 dir=faceGridCellDirection(CubeFace::PositiveZ,PlanetSurface::FaceResolution/2,PlanetSurface::FaceResolution/2,PlanetSurface::FaceResolution);
        feet=dir*(surfaceRead().surfaceBoundaryRadius(dir)+0.05f);
    }

    const auto initialFrame=planetSurface().surfaceFrame(feet);
    Vec3 forward=::elysium::normalize(initialFrame.forward*std::cos(yaw_)+initialFrame.right*std::sin(yaw_));
    Vec3 right=::elysium::normalize(cross(forward,initialFrame.up));
    const float moveForward=(IsKeyDown(KEY_W)?1.0f:0.0f)-(IsKeyDown(KEY_S)?1.0f:0.0f);
    const float moveRight=(IsKeyDown(KEY_D)?1.0f:0.0f)-(IsKeyDown(KEY_A)?1.0f:0.0f);
    Vec3 wish=forward*moveForward+right*moveRight;
    if (lengthSq(wish)>1.0f) wish=::elysium::normalize(wish);

    const bool moving=lengthSq(wish)>0.001f;
    const bool sprinting=IsKeyDown(KEY_LEFT_SHIFT) && ecs_.player().energy>0.5f;
    const float speed=sprinting?7.2f:4.6f;

    // Horizontal movement now preserves current radial depth instead of snapping
    // to the highest column. Collision samples the actual macro/micro volume, so
    // tunnels, overhangs and constructed interiors can be traversed.
    if (moving) {
        const float distance=speed*dt;
        const int steps=std::max(1,static_cast<int>(std::ceil(distance/0.20f)));
        const float stepDistance=distance/static_cast<float>(steps);
        for(int i=0;i<steps;++i) {
            const float radius=length(feet);
            const Vec3 candidateDir=::elysium::normalize(feet + wish*stepDistance);
            Vec3 candidate=candidateDir*radius;
            if (!surfaceRead().capsuleCollides(candidate)) {
                feet=candidate;
                continue;
            }
            // A bounded one-metre step-up keeps ordinary voxel terrain walkable
            // without reverting to the old height-field controller.
            if (sphericalGrounded_) {
                candidate=candidateDir*(radius+1.02f);
                if (!surfaceRead().capsuleCollides(candidate)) {
                    feet=candidate;
                    sphericalGrounded_=false;
                }
            }
        }
    }

    sphericalGrounded_=surfaceRead().groundedAt(feet,0.10f) && sphericalVerticalVelocity_<=0.0f;
    if (sphericalGrounded_ && IsKeyPressed(KEY_SPACE)) {
        sphericalGrounded_=false;
        sphericalVerticalVelocity_=6.0f;
    } else if (!sphericalGrounded_) {
        sphericalVerticalVelocity_-=14.0f*dt;
    } else {
        sphericalVerticalVelocity_=0.0f;
    }

    const float radialMove=sphericalVerticalVelocity_*dt;
    if (std::abs(radialMove)>0.00001f) {
        const int steps=std::max(1,static_cast<int>(std::ceil(std::abs(radialMove)/0.06f)));
        const float step=radialMove/static_cast<float>(steps);
        for(int i=0;i<steps;++i) {
            const Vec3 up=::elysium::normalize(feet);
            const Vec3 candidate=feet+up*step;
            if (surfaceRead().capsuleCollides(candidate)) {
                if (step<0.0f) sphericalGrounded_=true;
                sphericalVerticalVelocity_=0.0f;
                break;
            }
            feet=candidate;
            sphericalGrounded_=false;
        }
    }
    if (!sphericalGrounded_ && sphericalVerticalVelocity_<=0.0f && surfaceRead().groundedAt(feet,0.10f)) {
        sphericalGrounded_=true;
        sphericalVerticalVelocity_=0.0f;
    }

    const auto env=world().environment();
    const Vec3 suitCenter=feet+::elysium::normalize(feet)*0.9f;
    const auto atmosphere=surfaceBase().atmosphereAt(planetSurface(),suitCenter);
    float oxygenDrain=env.oxygenDrainPerSecond;
    if(atmosphere.breathable()) {
        oxygenDrain=-7.0f;
    } else if(oxygenDrain>0.0f && (atmosphere.pressure>0.0f || atmosphere.oxygen>0.0f)) {
        const float pressureShare=std::clamp(atmosphere.pressure/0.55f,0.0f,1.0f);
        const float oxygenShare=std::clamp(atmosphere.oxygen/0.45f,0.0f,1.0f);
        const float partialSupport=std::min(pressureShare,oxygenShare);
        oxygenDrain*=1.0f-0.75f*partialSupport;
    }
    ecs_.updateVitals(dt,oxygenDrain,env.hazardDamagePerSecond,sprinting,moving);
    if (IsKeyPressed(KEY_ESCAPE)) sphericalSurfaceMode_=false;
}

void Game::updateSphericalInteraction(float dt) {
    if (IsKeyPressed(KEY_F)) trySphericalAttack();
    if (IsKeyPressed(KEY_V)) {
        sculptMode_=!sculptMode_;
        miningProgress_=0.0f;
        sphericalMiningValid_=false;
        setMessage(sculptMode_ ? "SPHERICAL MICRO SCULPT // 6.25 cm edits live" : "SPHERICAL MACRO TOOL // mining/building",2.2f);
    }
    if (IsKeyPressed(KEY_E)) {
        if (!trySurfacePortalInteract() && !trySurfaceMachineInteract() && nearShip()) {
            resetPlayerMetersAtShip();
            setMessage("Ship life support restored suit reserves.");
        }
    }
    if (sculptMode_) {
        trySphericalMicroSculpt();
        return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (portalMode_) trySphericalPlacePortal();
        else if (machineMode_) trySphericalPlaceMachine();
        else trySphericalPlaceBlock();
    }
    trySphericalMine(dt);
}

void Game::updatePlayer(float dt) {
    const Vector2 mouse = GetMouseDelta();
    yaw_ += mouse.x * 0.00225f;
    pitch_ -= mouse.y * 0.00225f;
    pitch_ = std::clamp(pitch_, -1.48f, 1.48f);

    PlayerInput input{};
    input.moveForward = (IsKeyDown(KEY_W) ? 1.0f : 0.0f) - (IsKeyDown(KEY_S) ? 1.0f : 0.0f);
    input.moveRight = (IsKeyDown(KEY_D) ? 1.0f : 0.0f) - (IsKeyDown(KEY_A) ? 1.0f : 0.0f);
    input.forward = cameraForward();
    input.jump = IsKeyPressed(KEY_SPACE);
    input.sprint = IsKeyDown(KEY_LEFT_SHIFT);

    const auto pos = ecs_.player().position;
    const float hazard = world().localHazardAt(pos);
    const IVec3 playerCell{static_cast<int>(std::floor(pos.x)),static_cast<int>(std::floor(pos.y+0.9f)),static_cast<int>(std::floor(pos.z))};
    const bool oxygenated = base().oxygenatedAt(world(),playerCell);
    const float oxygenDrain = oxygenated ? -7.0f : world().environment().oxygenDrainPerSecond;
    ecs_.update(dt, input,
                [this](int x,int y,int z){ return world().isSolid(x,y,z); },
                oxygenDrain,
                hazard);

    if (IsKeyPressed(KEY_E)) {
        if (!tryMachineInteract() && nearShip()) {
            resetPlayerMetersAtShip();
            setMessage("Ship life support restored suit reserves.");
        }
    }
}

void Game::updateInteraction(float dt) {
    if (IsKeyPressed(KEY_V)) {
        sculptMode_ = !sculptMode_;
        miningProgress_ = 0.0f;
        miningCell_ = {-999,-999,-999};
        setMessage(sculptMode_ ? "MICRO SCULPT // 6.25 cm chisel enabled" : "MACRO TOOL // mining/building enabled", 2.0f);
    }
    if (IsKeyPressed(KEY_F)) tryAttack();
    if (sculptMode_) {
        tryMicroSculpt();
        return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        if (machineMode_) tryPlaceMachine(); else tryPlaceBlock();
    }
    tryMine(dt);
}

void Game::selectHotbar() {
    if (IsKeyPressed(KEY_ONE)) { selectedBlock_ = BlockType::Dirt; machineMode_=false; portalMode_=false; }
    if (IsKeyPressed(KEY_TWO)) { selectedBlock_ = BlockType::Stone; machineMode_=false; portalMode_=false; }
    if (IsKeyPressed(KEY_THREE)) { selectedBlock_ = BlockType::Planks; machineMode_=false; portalMode_=false; }
    if (IsKeyPressed(KEY_FOUR)) { selectedBlock_ = BlockType::SteelPlate; machineMode_=false; portalMode_=false; }
    if (IsKeyPressed(KEY_FIVE)) { selectedBlock_ = BlockType::RegistryBeacon; machineMode_=false; portalMode_=false; }
    if (IsKeyPressed(KEY_SIX)) { selectedMachine_=MachineType::BurnerGenerator; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_SEVEN)) { selectedMachine_=MachineType::BatteryBank; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_EIGHT)) { selectedMachine_=MachineType::AtmosphereUnit; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_NINE)) { selectedMachine_=MachineType::StorageCrate; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_K)) { selectedMachine_=MachineType::AirlockController; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_J)) { selectedMachine_=MachineType::SensorMast; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_U)) { selectedMachine_=MachineType::Turret; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_Y)) { selectedMachine_=MachineType::ShieldPylon; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_L)) { selectedMachine_=MachineType::LogicController; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_G)) { selectedMachine_=MachineType::Furnace; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_H)) { selectedMachine_=MachineType::AlloyCrucible; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_I)) { selectedMachine_=MachineType::Refinery; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_N)) { selectedMachine_=MachineType::NetworkStorage; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_Q)) { selectedMachine_=MachineType::Conveyor; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_R)) { selectedMachine_=MachineType::Sorter; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_X)) { selectedMachine_=MachineType::CargoLoader; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_Z)) { selectedMachine_=MachineType::Crusher; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_F1)) { selectedMachine_=MachineType::ChemicalVat; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_F2)) { selectedMachine_=MachineType::Fabricator; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_F3)) { selectedMachine_=MachineType::Extractor; machineMode_=true; portalMode_=false; }
    if (IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_RIGHT_BRACKET)) {
        constexpr int first=static_cast<int>(MachineType::BurnerGenerator);
        constexpr int last=static_cast<int>(MachineType::ArcSmelter);
        int value=static_cast<int>(selectedMachine_);
        value += IsKeyPressed(KEY_RIGHT_BRACKET) ? 1 : -1;
        if(value>last) value=first;
        if(value<first) value=last;
        selectedMachine_=static_cast<MachineType>(value);
        machineMode_=true;
        portalMode_=false;
        setMessage(std::string("MACHINE SELECT // ")+machineName(selectedMachine_),1.4f);
    }
    if (IsKeyPressed(KEY_B)) { selectedPortal_=SurfacePortalType::Door; portalMode_=true; machineMode_=false; }
    if (IsKeyPressed(KEY_ZERO)) { selectedPortal_=SurfacePortalType::Airlock; portalMode_=true; machineMode_=false; }
}

void Game::tryMine(float dt) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        miningProgress_ = 0.0f;
        miningCell_ = {-999,-999,-999};
        return;
    }

    const auto hit = world().raycast(cameraPosition(), cameraForward(), 7.0f);
    if (!hit || !blockProperties(hit->type).mineable) {
        miningProgress_ = 0.0f;
        return;
    }

    if (!(hit->cell == miningCell_)) {
        miningCell_ = hit->cell;
        miningProgress_ = 0.0f;
    }

    const auto& props = blockProperties(hit->type);
    if (toolTier_ < props.harvestTier) {
        miningProgress_ = 0.0f;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) setMessage("TOOL GATE // upgrade mining head at [C]");
        return;
    }

    const float mineSeconds = std::max(0.18f, props.hardness * 0.32f / (1.0f + 0.25f * static_cast<float>(toolTier_ - 1)));
    miningProgress_ += dt / mineSeconds;
    if (miningProgress_ >= 1.0f) {
        const BlockType mined = hit->type;
        const BlockType drop = miningDrop(mined);
        world().set(hit->cell.x, hit->cell.y, hit->cell.z, BlockType::Air);
        if (drop != BlockType::Air) ecs_.addItem(static_cast<int>(drop), 1);
        raiseSuspicion(props.suspicionOnMine);
        if (mined == BlockType::RegistryBeacon) {
            claimed_[static_cast<std::size_t>(currentPlanet_)] = false;
            setMessage("CLAIM STRUCK // structures and terrain edits remain persistent", 4.0f);
        } else {
            setMessage(std::string("Extracted ") + std::string(props.name));
        }
        miningProgress_ = 0.0f;
        miningCell_ = {-999,-999,-999};
    }
}

void Game::trySphericalMine(float dt) {
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        miningProgress_=0.0f;
        sphericalMiningValid_=false;
        return;
    }

    const auto hit=surfaceRead().raycast(sphericalCameraPosition(),sphericalCameraForward(),7.0f,0.08f);
    if (!hit.hit) {
        miningProgress_=0.0f;
        sphericalMiningValid_=false;
        return;
    }
    const BlockType type=planetSurface().get(hit.cell);
    const auto& props=blockProperties(type);
    if (!props.mineable) {
        miningProgress_=0.0f;
        sphericalMiningValid_=false;
        return;
    }
    if (!sphericalMiningValid_ || !(hit.cell==sphericalMiningCell_)) {
        sphericalMiningCell_=hit.cell;
        sphericalMiningValid_=true;
        miningProgress_=0.0f;
    }
    if (toolTier_<props.harvestTier) {
        miningProgress_=0.0f;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) setMessage("TOOL GATE // upgrade mining head on the legacy fabrication screen [C]");
        return;
    }

    const float mineSeconds=std::max(0.18f,props.hardness*0.32f/(1.0f+0.25f*static_cast<float>(toolTier_-1)));
    miningProgress_+=dt/mineSeconds;
    if (miningProgress_<1.0f) return;

    const bool wasPlaced=planetSurface().playerPlaced(hit.cell);
    std::uint64_t minedPortalId{};
    SurfacePortalType minedPortalType{SurfacePortalType::Door};
    for(const auto& portal:surfaceBase().portals()) {
        if(portal.anchor==hit.cell) { minedPortalId=portal.stableId; minedPortalType=portal.type; break; }
    }
    if(minedPortalId!=0) {
        surfaceBase().removePortal(planetSurface(),minedPortalId,true);
        ecs_.addItem(surfacePortalItemId(minedPortalType),1);
    } else {
        planetSurface().set(hit.cell,BlockType::Air);
        const BlockType drop=miningDrop(type);
        if (drop!=BlockType::Air) ecs_.addItem(static_cast<int>(drop),1);
    }
    if (!wasPlaced) raiseSuspicion(props.suspicionOnMine);
    if (type==BlockType::RegistryBeacon) {
        claimed_[static_cast<std::size_t>(currentPlanet_)]=false;
        setMessage("CLAIM STRUCK ON CUBE-SPHERE // planetary edits remain",3.4f);
    } else {
        setMessage(std::string("SPHERICAL EXTRACT // ")+std::string(props.name),1.5f);
    }
    miningProgress_=0.0f;
    sphericalMiningValid_=false;
}

void Game::trySphericalMicroSculpt() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const auto hit=surfaceRead().raycastMicro(sphericalCameraPosition(),sphericalCameraForward(),7.0f,0.012f);
    if (!hit.hit) return;
    const auto& props=blockProperties(hit.type);
    if (hit.type==BlockType::DoorPanel || hit.type==BlockType::AirlockPanel) {
        setMessage("PORTAL FRAME // remove with macro tool",1.4f);
        return;
    }
    if (!props.mineable) {
        setMessage("SPHERICAL MICRO // material cannot be cut",1.2f);
        return;
    }
    if (toolTier_<props.harvestTier) {
        setMessage("MICRO TOOL GATE // upgrade mining head at [C]",1.5f);
        return;
    }
    const bool wasPlaced=planetSurface().playerPlaced(hit.micro.cell);
    planetSurface().setMicro(hit.micro.cell,hit.micro.u,hit.micro.radial,hit.micro.v,BlockType::Air);
    if (!wasPlaced) raiseSuspicion(props.suspicionOnMine/static_cast<float>(MicroBrick::CellCount));
    setMessage("SPHERICAL MICRO CHISEL // persistent 6.25 cm edit",1.2f);
}

void Game::trySphericalPlaceMachine() {
    const auto hit=surfaceRead().raycast(sphericalCameraPosition(),sphericalCameraForward(),7.0f,0.06f);
    if (!hit.hit) return;
    const SurfaceCellAddress target=hit.previous;
    if (!planetSurface().radialInBounds(target.radial) || surfaceRead().solidAt(planetSurface().cellCenterPosition(target))) return;
    if (surfaceBase().occupied(target) || surfaceBase().portalOccupied(target)) { setMessage("Surface machine anchor occupied."); return; }
    const Vec3 center=planetSurface().cellCenterPosition(target);
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const Vec3 suitCenter=feet+::elysium::normalize(feet)*0.9f;
    if (length(center-suitCenter)<1.05f) { setMessage("Machine placement obstructed by suit volume."); return; }
    const int item=machineItemId(selectedMachine_);
    if (!ecs_.consumeItem(item,1)) { setMessage(std::string("No ")+machineName(selectedMachine_)+" kit in inventory."); return; }
    surfaceBase().place(selectedMachine_,target);
    surfaceBase().update(planetSurface(),0.0f);
    raiseSuspicion(0.35f);
    setMessage(std::string("SURFACE MACHINE // ")+machineName(selectedMachine_),2.5f);
}

bool Game::trySurfaceMachineInteract() {
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const Vec3 suitCenter=feet+::elysium::normalize(feet)*0.9f;
    if (auto* gen=surfaceBase().nearest(planetSurface(),MachineType::BurnerGenerator,suitCenter,2.8f)) {
        if (ecs_.consumeItem(static_cast<int>(BlockType::CoalOre),1)) {
            gen->fuelSeconds+=60.0f;
            setMessage("Surface Burner fueled // +60 s coal burn",2.5f);
        } else setMessage("Surface Burner requires Coal Ore.",2.0f);
        return true;
    }
    // Prototype route-authoring path for the standalone client. The persistent
    // contract remains stable-ID + owner-address; this convenience interaction
    // merely chooses nearby endpoints and calls the same validated industry API.
    auto tryConfigureNearbyLogistics=[&]()->bool {
        SurfaceMachineObject* transport=nullptr;
        for(const auto type:{MachineType::Conveyor,MachineType::Sorter,MachineType::CargoLoader}) {
            if(auto* candidate=surfaceBase().nearest(planetSurface(),type,suitCenter,2.8f)) { transport=candidate; break; }
        }
        if(!transport) return false;
        if(!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_RIGHT_CONTROL)) return false;

        std::vector<const SurfaceMachineObject*> candidates;
        for(const auto& object:surfaceBase().objects()) {
            if(object.stableId==transport->stableId) continue;
            switch(object.type) {
                case MachineType::StorageCrate:
                case MachineType::Furnace:
                case MachineType::AlloyCrucible:
                case MachineType::Refinery:
                case MachineType::NetworkStorage:
                case MachineType::Conveyor:
                case MachineType::Sorter:
                case MachineType::CargoLoader:
                case MachineType::Crusher:
                case MachineType::ChemicalVat:
                case MachineType::Fabricator:
                case MachineType::Extractor:
                    break;
                default: continue;
            }
            const float d=lengthSq(planetSurface().cellCenterPosition(object.anchor)-planetSurface().cellCenterPosition(transport->anchor));
            if(d<=SurfaceIndustrySystem::MaxLogisticsLinkDistance*SurfaceIndustrySystem::MaxLogisticsLinkDistance)
                candidates.push_back(&object);
        }
        std::sort(candidates.begin(),candidates.end(),[&](const auto* a,const auto* b){
            const Vec3 t=planetSurface().cellCenterPosition(transport->anchor);
            const float da=lengthSq(planetSurface().cellCenterPosition(a->anchor)-t);
            const float db=lengthSq(planetSurface().cellCenterPosition(b->anchor)-t);
            if(std::abs(da-db)>1e-5f) return da<db;
            return a->stableId<b->stableId;
        });
        const std::size_t needed=transport->type==MachineType::Sorter?3U:2U;
        if(candidates.size()<needed) {
            setMessage("LOGISTICS LINK // place nearby source/target inventories first",2.8f);
            return true;
        }
        int filter=0;
        std::uint64_t alternate=0;
        if(transport->type==MachineType::Sorter) {
            alternate=candidates[2]->stableId;
            filter=!candidates[0]->inventory.empty()?candidates[0]->inventory.front().itemId:static_cast<int>(BlockType::CopperOre);
        }
        if(surfaceIndustry_.configureLogisticsLink(surfaceBase(),planetSurface(),transport->stableId,
                                                   candidates[0]->stableId,candidates[1]->stableId,alternate,filter)) {
            setMessage(std::string("LOGISTICS ROUTE // ")+machineName(transport->type)+" linked by stable IDs",3.0f);
        } else setMessage("LOGISTICS ROUTE // endpoint validation rejected",2.5f);
        return true;
    };
    if(tryConfigureNearbyLogistics()) return true;

    auto industryInteract=[&](MachineType type,const std::vector<int>& acceptedItems,const char* label)->bool {
        auto* machine=surfaceBase().nearest(planetSurface(),type,suitCenter,2.8f);
        if(!machine) return false;
        if(IsKeyDown(KEY_LEFT_SHIFT)) {
            if(machine->inventory.empty()) {
                setMessage(std::string(label)+" inventory empty.",1.8f);
                return true;
            }
            const int itemId=machine->inventory.front().itemId;
            if(surfaceIndustry_.extract(surfaceBase(),machine->stableId,itemId,1)==1) {
                ecs_.addItem(itemId,1);
                setMessage(std::string(label)+" // withdrew 1 "+industryItemName(itemId),2.0f);
            }
            return true;
        }
        for(const int itemId:acceptedItems) {
            if(ecs_.inventoryCount(itemId)<=0) continue;
            if(!ecs_.consumeItem(itemId,1)) continue;
            if(surfaceIndustry_.insert(surfaceBase(),machine->stableId,itemId,1)) {
                setMessage(std::string(label)+" // loaded 1 "+industryItemName(itemId),2.0f);
            } else {
                ecs_.addItem(itemId,1);
                setMessage(std::string(label)+" inventory full.",1.8f);
            }
            return true;
        }
        setMessage(std::string(label)+" // no compatible material; hold Shift+E to withdraw",2.4f);
        return true;
    };
    if(industryInteract(MachineType::Furnace,{static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::CoalOre)},"FURNACE")) return true;
    if(industryInteract(MachineType::AlloyCrucible,{static_cast<int>(IndustryItemId::CopperIngot),static_cast<int>(IndustryItemId::TinIngot),static_cast<int>(IndustryItemId::IronIngot),static_cast<int>(IndustryItemId::Carbon)},"ALLOY CRUCIBLE")) return true;
    if(industryInteract(MachineType::Crusher,{static_cast<int>(BlockType::Stone),static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre)},"CRUSHER")) return true;
    if(industryInteract(MachineType::Refinery,{static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre),static_cast<int>(IndustryItemId::CopperConcentrate),static_cast<int>(IndustryItemId::TinConcentrate),static_cast<int>(IndustryItemId::IronConcentrate)},"REFINERY")) return true;
    if(industryInteract(MachineType::ChemicalVat,{static_cast<int>(IndustryItemId::StoneAggregate),static_cast<int>(IndustryItemId::Carbon)},"CHEMICAL VAT")) return true;
    if(industryInteract(MachineType::Fabricator,{static_cast<int>(IndustryItemId::CopperIngot),static_cast<int>(IndustryItemId::SteelIngot),static_cast<int>(IndustryItemId::BronzeIngot),static_cast<int>(IndustryItemId::Carbon),static_cast<int>(IndustryItemId::StoneAggregate),static_cast<int>(IndustryItemId::Sealant),static_cast<int>(IndustryItemId::CopperWire),static_cast<int>(IndustryItemId::SteelFrame),static_cast<int>(IndustryItemId::Actuator),static_cast<int>(IndustryItemId::ControlCircuit),static_cast<int>(BlockType::Planks)},"FABRICATOR")) return true;
    if(industryInteract(MachineType::Extractor,{},"EXTRACTOR")) return true;
    if(industryInteract(MachineType::Conveyor,{static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre),static_cast<int>(IndustryItemId::CopperConcentrate),static_cast<int>(IndustryItemId::TinConcentrate),static_cast<int>(IndustryItemId::IronConcentrate)},"CONVEYOR")) return true;
    if(industryInteract(MachineType::Sorter,{static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre),static_cast<int>(IndustryItemId::CopperIngot),static_cast<int>(IndustryItemId::SteelIngot)},"SORTER")) return true;
    if(industryInteract(MachineType::CargoLoader,{static_cast<int>(BlockType::SteelPlate),static_cast<int>(IndustryItemId::SteelIngot),static_cast<int>(IndustryItemId::RepairKit),static_cast<int>(IndustryItemId::TurretAmmo)},"CARGO LOADER")) return true;
    if(industryInteract(MachineType::NetworkStorage,{static_cast<int>(BlockType::CoalOre),static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::Stone),static_cast<int>(BlockType::Planks),static_cast<int>(BlockType::SteelPlate),static_cast<int>(IndustryItemId::CopperIngot),static_cast<int>(IndustryItemId::TinIngot),static_cast<int>(IndustryItemId::IronIngot),static_cast<int>(IndustryItemId::Carbon),static_cast<int>(IndustryItemId::BronzeIngot),static_cast<int>(IndustryItemId::SteelIngot),static_cast<int>(IndustryItemId::StoneAggregate),static_cast<int>(IndustryItemId::CopperConcentrate),static_cast<int>(IndustryItemId::TinConcentrate),static_cast<int>(IndustryItemId::IronConcentrate),static_cast<int>(IndustryItemId::Sealant),static_cast<int>(IndustryItemId::CopperWire),static_cast<int>(IndustryItemId::SteelFrame),static_cast<int>(IndustryItemId::Actuator),static_cast<int>(IndustryItemId::ControlCircuit),static_cast<int>(IndustryItemId::SensorPackage),static_cast<int>(IndustryItemId::TurretAmmo),static_cast<int>(IndustryItemId::RepairKit),static_cast<int>(IndustryItemId::FilterCartridge),static_cast<int>(IndustryItemId::MachineCasing),static_cast<int>(IndustryItemId::CompositePanel)},"NETWORK STORAGE")) return true;
    if(auto* controller=surfaceBase().nearest(planetSurface(),MachineType::AirlockController,suitCenter,3.0f)) {
        auto* assembly=surfaceBase().airlockForController(controller->stableId);
        if(!assembly) {
            std::vector<const SurfacePortalObject*> candidates;
            for(const auto& portal:surfaceBase().portals()) {
                if(portal.type!=SurfacePortalType::Airlock || surfaceBase().airlockForPortal(portal.stableId)) continue;
                if(length(planetSurface().cellCenterPosition(portal.anchor)-planetSurface().cellCenterPosition(controller->anchor))<=6.0f)
                    candidates.push_back(&portal);
            }
            std::sort(candidates.begin(),candidates.end(),[this](const auto* a,const auto* b){
                if(a->anchor.radial!=b->anchor.radial) return a->anchor.radial<b->anchor.radial;
                return a->stableId<b->stableId;
            });
            if(candidates.size()<2) {
                setMessage("Airlock Controller needs two nearby unpaired Airlock portals.",3.0f);
                return true;
            }
            const auto id=surfaceBase().createAirlockAssembly(planetSurface(),controller->stableId,candidates[0]->stableId,candidates[1]->stableId,controller->anchor,1.0f,1.0f);
            if(id==0) setMessage("Airlock pairing rejected.",2.5f);
            else setMessage("AIRLOCK INTERLOCK PAIRED // E cycles exterior/interior",3.2f);
            return true;
        }
        if(assembly->state==SurfaceAirlockState::Depressurizing || assembly->state==SurfaceAirlockState::Pressurizing) {
            setMessage(std::string("AIRLOCK // ")+surfaceAirlockStateName(assembly->state),1.8f);
            return true;
        }
        const auto destination=assembly->state==SurfaceAirlockState::ExteriorOpen
            ? SurfaceAirlockDestination::Interior
            : SurfaceAirlockDestination::Exterior;
        if(surfaceBase().requestAirlockCycle(planetSurface(),assembly->stableId,destination))
            setMessage(destination==SurfaceAirlockDestination::Exterior?"AIRLOCK CYCLE // depressurizing":"AIRLOCK CYCLE // pressurizing",2.3f);
        else setMessage("AIRLOCK CONTROLLER FAULT",2.3f);
        return true;
    }
    if(auto* turret=surfaceBase().nearest(planetSurface(),MachineType::Turret,suitCenter,2.8f)) {
        if(ecs_.consumeItem(static_cast<int>(BlockType::SteelPlate),1)) {
            turret->ammo+=12;
            setMessage("TURRET MAGAZINE // +12 rounds from 1 Steel Plate",2.5f);
        } else setMessage("Turret reload requires 1 Steel Plate.",2.0f);
        return true;
    }
    if(auto* logic=surfaceBase().nearest(planetSurface(),MachineType::LogicController,suitCenter,3.0f)) {
        int existing=0;
        for(const auto& rule:surfaceBase().automationRules()) if(rule.controllerMachineId==logic->stableId) ++existing;
        if(existing>0) {
            setMessage("LOGIC CONTROLLER // automation rules already configured",2.5f);
            return true;
        }
        const Vec3 logicPos=planetSurface().cellCenterPosition(logic->anchor);
        SurfaceMachineObject* sensor=surfaceBase().nearest(planetSurface(),MachineType::SensorMast,logicPos,12.0f);
        SurfaceMachineObject* turret=surfaceBase().nearest(planetSurface(),MachineType::Turret,logicPos,12.0f);
        if(!sensor || !turret) {
            setMessage("Logic Controller needs a Sensor Mast + Turret within 12 m.",3.0f);
            return true;
        }
        turret->enabled=false;
        const auto ruleId=surfaceBase().createAutomationRule(logic->stableId,SurfaceAutomationTrigger::HostilesDetected,
                                                              sensor->stableId,0.0f,SurfaceAutomationAction::EnableMachine,turret->stableId);
        if(ruleId==0) { setMessage("Automation rule creation rejected.",2.5f); return true; }
        if(auto* portal=surfaceBase().nearestPortal(planetSurface(),logicPos,12.0f))
            surfaceBase().createAutomationRule(logic->stableId,SurfaceAutomationTrigger::HostilesDetected,
                                               sensor->stableId,0.0f,SurfaceAutomationAction::ClosePortal,portal->stableId);
        setMessage("AUTOMATION // hostiles enable turret + close nearest portal",3.2f);
        return true;
    }
    return false;
}

void Game::trySphericalPlacePortal() {
    const auto hit=surfaceRead().raycast(sphericalCameraPosition(),sphericalCameraForward(),7.0f,0.06f);
    if(!hit.hit) return;
    const SurfaceCellAddress target=hit.previous;
    if(!planetSurface().radialInBounds(target.radial) || planetSurface().get(target)!=BlockType::Air) return;
    if(surfaceBase().occupied(target) || surfaceBase().portalOccupied(target)) { setMessage("Surface anchor occupied."); return; }
    const Vec3 center=planetSurface().cellCenterPosition(target);
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const Vec3 suitCenter=feet+normalize(feet)*0.9f;
    if(length(center-suitCenter)<1.05f) { setMessage("Portal placement obstructed by suit volume."); return; }
    const int item=surfacePortalItemId(selectedPortal_);
    if(!ecs_.consumeItem(item,1)) { setMessage(std::string("No ")+surfacePortalName(selectedPortal_)+" kit in inventory."); return; }
    const auto id=surfaceBase().placePortal(planetSurface(),selectedPortal_,target,false);
    if(id==0) { ecs_.addItem(item,1); setMessage("Portal placement rejected."); return; }
    raiseSuspicion(selectedPortal_==SurfacePortalType::Airlock?0.28f:0.18f);
    setMessage(std::string("SURFACE ")+surfacePortalName(selectedPortal_)+" // closed + sealed",2.5f);
}

bool Game::trySurfacePortalInteract() {
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const Vec3 suitCenter=feet+normalize(feet)*0.9f;
    auto* portal=surfaceBase().nearestPortal(planetSurface(),suitCenter,2.6f);
    if(!portal) return false;
    const bool opening=!portal->open;
    if(!surfaceBase().setPortalOpen(planetSurface(),portal->stableId,opening)) {
        if(surfaceBase().airlockForPortal(portal->stableId)) setMessage("INTERLOCKED AIRLOCK // use its controller",2.1f);
        return true;
    }
    setMessage(std::string(surfacePortalName(portal->type))+(opening?" OPEN // atmosphere breach possible":" CLOSED // seal restored"),2.2f);
    return true;
}

void Game::trySphericalPlaceBlock() {
    const auto hit=surfaceRead().raycast(sphericalCameraPosition(),sphericalCameraForward(),7.0f,0.08f);
    if (!hit.hit) return;
    const auto& props=blockProperties(selectedBlock_);
    if (!props.placeable) return;
    const SurfaceCellAddress target=hit.previous;
    if (!planetSurface().radialInBounds(target.radial) || planetSurface().get(target)!=BlockType::Air || target==hit.cell) return;

    const Vec3 center=planetSurface().cellCenterPosition(target);
    const Vec3 suitCenter=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]+normalize(sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)])*0.9f;
    if (length(center-suitCenter)<1.15f) {
        setMessage("Placement obstructed by suit volume.");
        return;
    }
    const int id=static_cast<int>(selectedBlock_);
    if (!ecs_.consumeItem(id,1)) {
        setMessage(std::string("No ")+std::string(props.name)+" in inventory.");
        return;
    }
    planetSurface().set(target,selectedBlock_,true);
    raiseSuspicion(0.15f);
    if (selectedBlock_==BlockType::RegistryBeacon) {
        claimed_[static_cast<std::size_t>(currentPlanet_)]=true;
        suspicion_=std::max(suspicion_,claimFloor());
        setMessage("REGISTRY BEACON ONLINE // cube-sphere claim filed",4.0f);
    }
}

void Game::tryMicroSculpt() {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const auto hit = world().raycastMicro(cameraPosition(), cameraForward(), 7.0f);
    if (!hit) return;
    const auto& props = blockProperties(hit->type);
    if (!props.mineable) {
        setMessage("MICRO SCULPT // material cannot be cut", 1.2f);
        return;
    }
    if (toolTier_ < props.harvestTier) {
        setMessage("MICRO TOOL GATE // upgrade mining head at [C]", 1.5f);
        return;
    }

    world().setMicroGlobal(hit->globalMicro.x, hit->globalMicro.y, hit->globalMicro.z, BlockType::Air);
    // Micro-chiseling deliberately yields no item: a 1/4096 m^3 chip must not
    // become a full block and create a duplication exploit.
    raiseSuspicion(props.suspicionOnMine / static_cast<float>(MicroBrick::CellCount));
    setMessage("MICRO CHISEL // persistent 6.25 cm edit (no full-block drop)", 1.2f);
}

void Game::tryPlaceBlock() {
    const auto hit = world().raycast(cameraPosition(), cameraForward(), 7.0f);
    if (!hit) return;
    const auto& props = blockProperties(selectedBlock_);
    if (!props.placeable) return;

    const IVec3 c{hit->cell.x + hit->normal.x, hit->cell.y + hit->normal.y, hit->cell.z + hit->normal.z};
    if (!world().inBounds(c.x,c.y,c.z) || world().get(c.x,c.y,c.z) != BlockType::Air) return;

    const Vec3 p = ecs_.player().position;
    const Vec3 blockCenter{c.x + 0.5f, c.y + 0.5f, c.z + 0.5f};
    if (length(blockCenter - (p + Vec3{0,0.9f,0})) < 1.05f) {
        setMessage("Placement obstructed by suit volume.");
        return;
    }

    const int id = static_cast<int>(selectedBlock_);
    if (!ecs_.consumeItem(id, 1)) {
        setMessage(std::string("No ") + std::string(props.name) + " in inventory.");
        return;
    }

    world().set(c.x,c.y,c.z, selectedBlock_);
    raiseSuspicion(0.15f); // proposed activity pressure: tuning value for this prototype
    if (selectedBlock_ == BlockType::RegistryBeacon) {
        claimed_[static_cast<std::size_t>(currentPlanet_)] = true;
        suspicion_ = std::max(suspicion_, claimFloor());
        setMessage("REGISTRY BEACON ONLINE // planetary claim filed", 4.0f);
    }
}


void Game::tryPlaceMachine() {
    const auto hit=world().raycast(cameraPosition(),cameraForward(),7.0f);
    if(!hit) return;
    const IVec3 c{hit->cell.x+hit->normal.x,hit->cell.y+hit->normal.y,hit->cell.z+hit->normal.z};
    if(!world().inBounds(c.x,c.y,c.z) || world().isSolid(c.x,c.y,c.z)) return;
    for(const auto& o:base().objects()) if(o.anchor==c) { setMessage("Machine anchor occupied."); return; }
    const Vec3 p=ecs_.player().position;
    const Vec3 center{c.x+0.5f,c.y+0.5f,c.z+0.5f};
    if(length(center-(p+Vec3{0,0.9f,0}))<1.0f){setMessage("Machine placement obstructed by suit volume.");return;}
    const int item=machineItemId(selectedMachine_);
    if(!ecs_.consumeItem(item,1)){setMessage(std::string("No ")+machineName(selectedMachine_)+" kit in inventory.");return;}
    base().place(selectedMachine_,c);
    raiseSuspicion(0.35f);
    setMessage(std::string("PLACED // ")+machineName(selectedMachine_),2.5f);
}

bool Game::tryMachineInteract() {
    if(auto* gen=base().nearest(MachineType::BurnerGenerator,ecs_.player().position,2.8f)) {
        if(ecs_.consumeItem(static_cast<int>(BlockType::CoalOre),1)) {
            gen->fuelSeconds += 60.0f;
            setMessage("Burner Generator fueled // +60 s coal burn",2.5f);
        } else setMessage("Burner Generator requires Coal Ore.",2.0f);
        return true;
    }
    return false;
}

void Game::tryAttack() {
    const auto result = ecs_.attackRay(cameraPosition(), cameraForward(), 18.0f, 18.0f);
    if (result.hit) {
        if (result.killed) {
            ecs_.addItem(static_cast<int>(BlockType::SteelPlate), 1);
            setMessage("Imperial drone neutralized // salvage +1 Steel Plate");
        } else setMessage("Drone hit.", 1.0f);
    }
}

void Game::trySphericalAttack() {
    const auto result=ecs_.attackRay(sphericalCameraPosition(),sphericalCameraForward(),22.0f,18.0f,ActorDomain::PlanetSurface);
    if (!result.hit) return;
    if (result.killed) {
        ecs_.addItem(static_cast<int>(BlockType::SteelPlate),1);
        setMessage("SURFACE DRONE NEUTRALIZED // salvage +1 Steel Plate",2.0f);
    } else setMessage("Surface drone hit.",1.0f);
}

void Game::tryCraft(int recipe) {
    auto have = [this](BlockType t, int n){ return ecs_.inventoryCount(static_cast<int>(t)) >= n; };
    auto use = [this](BlockType t, int n){ return ecs_.consumeItem(static_cast<int>(t), n); };
    auto haveItem = [this](IndustryItemId id, int n){ return ecs_.inventoryCount(static_cast<int>(id)) >= n; };
    auto useItem = [this](IndustryItemId id, int n){ return ecs_.consumeItem(static_cast<int>(id), n); };

    if (recipe == 1) {
        if (toolTier_ >= 2) { setMessage("Bronze mining head already installed."); return; }
        if (have(BlockType::CopperOre,3) && have(BlockType::TinOre,1)) {
            use(BlockType::CopperOre,3); use(BlockType::TinOre,1); toolTier_ = 2;
            setMessage("Bronze mining head installed // Tier 2");
        } else setMessage("Need 3 Copper Ore + 1 Tin Ore.");
    } else if (recipe == 2) {
        if (toolTier_ >= 3) { setMessage("Steel mining head already installed."); return; }
        if (toolTier_ >= 2 && have(BlockType::IronOre,4) && have(BlockType::CoalOre,2)) {
            use(BlockType::IronOre,4); use(BlockType::CoalOre,2); toolTier_ = 3;
            setMessage("Steel mining head installed // Tier 3");
        } else setMessage("Need Tier 2 + 4 Iron Ore + 2 Coal Ore.");
    } else if (recipe == 3) {
        if (have(BlockType::IronOre,2) && have(BlockType::CoalOre,1)) {
            use(BlockType::IronOre,2); use(BlockType::CoalOre,1);
            ecs_.addItem(static_cast<int>(BlockType::SteelPlate),1);
            setMessage("Fabricated 1 Steel Plate.");
        } else setMessage("Need 2 Iron Ore + 1 Coal Ore.");
    } else if (recipe == 4) {
        if (have(BlockType::SteelPlate,4) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,4); use(BlockType::CopperOre,2);
            ecs_.addItem(static_cast<int>(BlockType::RegistryBeacon),1);
            setMessage("Fabricated Registry Beacon.");
        } else setMessage("Need 4 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 5) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::BurnerGenerator),1);
            setMessage("Fabricated Burner Generator kit.");
        } else setMessage("Need 2 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 6) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,3)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,3);
            ecs_.addItem(machineItemId(MachineType::BatteryBank),1);
            setMessage("Fabricated Battery Bank kit.");
        } else setMessage("Need 2 Steel Plate + 3 Copper Ore.");
    } else if (recipe == 7) {
        if (have(BlockType::SteelPlate,3) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,3); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::AtmosphereUnit),1);
            setMessage("Fabricated Atmosphere Unit kit.");
        } else setMessage("Need 3 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 8) {
        if (have(BlockType::SteelPlate,1) && have(BlockType::Planks,4)) {
            use(BlockType::SteelPlate,1); use(BlockType::Planks,4);
            ecs_.addItem(machineItemId(MachineType::StorageCrate),1);
            setMessage("Fabricated Storage Crate kit.");
        } else setMessage("Need 1 Steel Plate + 4 Planks.");
    } else if (recipe == 9) {
        if (have(BlockType::SteelPlate,1) && have(BlockType::Planks,2)) {
            use(BlockType::SteelPlate,1); use(BlockType::Planks,2);
            ecs_.addItem(surfacePortalItemId(SurfacePortalType::Door),1);
            setMessage("Fabricated Door kit.");
        } else setMessage("Need 1 Steel Plate + 2 Planks.");
    } else if (recipe == 10) {
        if (have(BlockType::SteelPlate,3) && have(BlockType::CopperOre,1)) {
            use(BlockType::SteelPlate,3); use(BlockType::CopperOre,1);
            ecs_.addItem(surfacePortalItemId(SurfacePortalType::Airlock),1);
            setMessage("Fabricated Airlock kit.");
        } else setMessage("Need 3 Steel Plate + 1 Copper Ore.");
    } else if (recipe == 11) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::AirlockController),1);
            setMessage("Fabricated Airlock Controller kit.");
        } else setMessage("Need 2 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 12) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::SensorMast),1);
            setMessage("Fabricated Sensor Mast kit.");
        } else setMessage("Need 2 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 13) {
        if (have(BlockType::SteelPlate,4) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,4); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::Turret),1);
            setMessage("Fabricated Turret kit.");
        } else setMessage("Need 4 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 14) {
        if (have(BlockType::SteelPlate,6) && have(BlockType::CopperOre,3)) {
            use(BlockType::SteelPlate,6); use(BlockType::CopperOre,3);
            ecs_.addItem(machineItemId(MachineType::ShieldPylon),1);
            setMessage("Fabricated Shield Pylon kit.");
        } else setMessage("Need 6 Steel Plate + 3 Copper Ore.");
    } else if (recipe == 15) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,3)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,3);
            ecs_.addItem(machineItemId(MachineType::LogicController),1);
            setMessage("Fabricated Logic Controller kit.");
        } else setMessage("Need 2 Steel Plate + 3 Copper Ore.");
    } else if (recipe == 16) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CoalOre,1)) {
            use(BlockType::SteelPlate,2); use(BlockType::CoalOre,1);
            ecs_.addItem(machineItemId(MachineType::Furnace),1);
            setMessage("Fabricated Furnace kit.");
        } else setMessage("Need 2 Steel Plate + 1 Coal Ore.");
    } else if (recipe == 17) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::AlloyCrucible),1);
            setMessage("Fabricated Alloy Crucible kit.");
        } else setMessage("Need 2 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 18) {
        if (have(BlockType::SteelPlate,4) && have(BlockType::CopperOre,3)) {
            use(BlockType::SteelPlate,4); use(BlockType::CopperOre,3);
            ecs_.addItem(machineItemId(MachineType::Refinery),1);
            setMessage("Fabricated Refinery kit.");
        } else setMessage("Need 4 Steel Plate + 3 Copper Ore.");
    } else if (recipe == 19) {
        if (have(BlockType::SteelPlate,2) && have(BlockType::CopperOre,2)) {
            use(BlockType::SteelPlate,2); use(BlockType::CopperOre,2);
            ecs_.addItem(machineItemId(MachineType::NetworkStorage),1);
            setMessage("Fabricated Network Storage kit.");
        } else setMessage("Need 2 Steel Plate + 2 Copper Ore.");
    } else if (recipe == 20) {
        if (haveItem(IndustryItemId::SteelFrame,1) && haveItem(IndustryItemId::CopperWire,1)) {
            useItem(IndustryItemId::SteelFrame,1); useItem(IndustryItemId::CopperWire,1);
            ecs_.addItem(machineItemId(MachineType::Conveyor),2);
            setMessage("Fabricated 2 Conveyor kits.");
        } else setMessage("Need 1 Steel Frame + 1 Copper Wire.");
    } else if (recipe == 21) {
        if (haveItem(IndustryItemId::SteelFrame,1) && haveItem(IndustryItemId::ControlCircuit,1)) {
            useItem(IndustryItemId::SteelFrame,1); useItem(IndustryItemId::ControlCircuit,1);
            ecs_.addItem(machineItemId(MachineType::Sorter),1);
            setMessage("Fabricated Sorter kit.");
        } else setMessage("Need 1 Steel Frame + 1 Control Circuit.");
    } else if (recipe == 22) {
        if (haveItem(IndustryItemId::SteelFrame,2) && haveItem(IndustryItemId::Actuator,1)) {
            useItem(IndustryItemId::SteelFrame,2); useItem(IndustryItemId::Actuator,1);
            ecs_.addItem(machineItemId(MachineType::CargoLoader),1);
            setMessage("Fabricated Cargo Loader kit.");
        } else setMessage("Need 2 Steel Frames + 1 Actuator.");
    } else if (recipe == 23) {
        if (haveItem(IndustryItemId::SteelFrame,1) && haveItem(IndustryItemId::Actuator,1)) {
            useItem(IndustryItemId::SteelFrame,1); useItem(IndustryItemId::Actuator,1);
            ecs_.addItem(machineItemId(MachineType::Crusher),1);
            setMessage("Fabricated Crusher kit.");
        } else setMessage("Need 1 Steel Frame + 1 Actuator.");
    } else if (recipe == 24) {
        if (haveItem(IndustryItemId::SteelFrame,1) && haveItem(IndustryItemId::Sealant,2)) {
            useItem(IndustryItemId::SteelFrame,1); useItem(IndustryItemId::Sealant,2);
            ecs_.addItem(machineItemId(MachineType::ChemicalVat),1);
            setMessage("Fabricated Chemical Vat kit.");
        } else setMessage("Need 1 Steel Frame + 2 Sealant.");
    } else if (recipe == 25) {
        if (haveItem(IndustryItemId::MachineCasing,1) && haveItem(IndustryItemId::ControlCircuit,1) && haveItem(IndustryItemId::Actuator,1)) {
            useItem(IndustryItemId::MachineCasing,1); useItem(IndustryItemId::ControlCircuit,1); useItem(IndustryItemId::Actuator,1);
            ecs_.addItem(machineItemId(MachineType::Fabricator),1);
            setMessage("Fabricated Fabricator kit.");
        } else setMessage("Need Machine Casing + Control Circuit + Actuator.");
    } else if (recipe == 26) {
        if (haveItem(IndustryItemId::MachineCasing,1) && haveItem(IndustryItemId::Actuator,2) && haveItem(IndustryItemId::SensorPackage,1)) {
            useItem(IndustryItemId::MachineCasing,1); useItem(IndustryItemId::Actuator,2); useItem(IndustryItemId::SensorPackage,1);
            ecs_.addItem(machineItemId(MachineType::Extractor),1);
            setMessage("Fabricated Extractor kit.");
        } else setMessage("Need Machine Casing + 2 Actuators + Sensor Package.");
    }
}

void Game::updateEmpire(float dt) {
    auto& director=surfaceSiege();

    // Stable-ID reconciliation keeps the enforcement director independent of
    // EnTT entity values and combat implementation details.
    std::vector<std::uint64_t> liveIds;
    const ActorDomain domain=sphericalSurfaceMode_?ActorDomain::PlanetSurface:ActorDomain::Planar;
    for(const auto& enemy:ecs_.enemies(domain)) liveIds.push_back(enemy.stableId);
    const auto phaseBeforeReconcile=director.state().phase;
    director.reconcileLiveEnemies(liveIds);

    if(phaseBeforeReconcile!=director.state().phase && director.state().phase==RegisterActionPhase::Cleared) {
        const bool wasRegister=director.state().type==ImperialEnforcementType::RegisterAction;
        if(wasRegister) {
            suspicion_=std::max(claimFloor(),suspicion_-25.0f); // prototype tuning: "toward structural floor"
            favor_=std::min(100.0f,favor_+4.0f);
            ecs_.addItem(static_cast<int>(BlockType::SteelPlate),6);
            setMessage("REGISTER ACTION CLEARED // salvage secured // file pressure receding",6.0f);
        } else setMessage("IMPERIAL PATROL CLEARED",3.5f);
        director.clearTerminalState();
    }

    if(director.state().phase==RegisterActionPhase::Failed) {
        setMessage("REGISTER ACTION FAILED // claim struck // structures remain",6.0f);
        director.clearTerminalState();
    }

    // The normal Suspicion dispatch roll remains the trigger. The director owns
    // what that successful roll means at Noted/Marked/Hunted attention bands.
    dispatchTimer_ += dt;
    if(!director.active() && !director.terminal() && suspicion_>=25.0f && dispatchTimer_>=8.0f) {
        dispatchTimer_=0.0f;
        const float chance=std::min(0.40f,(suspicion_-25.0f)/200.0f);
        const float roll=hash01(galaxySeed_,static_cast<int>(dispatchRollCounter_++),currentPlanet_,0,0x44495350ULL);
        if(roll<chance && director.requestEnforcement(suspicion_,claimed_[static_cast<std::size_t>(currentPlanet_)])) {
            if(director.state().type==ImperialEnforcementType::RegisterAction) {
                setMessage(std::string("REGISTER ACTION ANNOUNCED // ")+imperialAttentionBandName(director.state().band)+
                           " // ~10 MINUTES TO CONTACT",6.0f);
            } else setMessage("IMPERIAL PATROL DISPATCHED // signature acquired",4.0f);
        }
    }

    const auto phaseBeforeUpdate=director.state().phase;
    const int waveBefore=director.state().waveIndex;
    auto spawns=director.update(dt,claimed_[static_cast<std::size_t>(currentPlanet_)],
                               claimed_[static_cast<std::size_t>(currentPlanet_)]);
    if(director.state().phase==RegisterActionPhase::Failed && phaseBeforeUpdate!=RegisterActionPhase::Failed) {
        setMessage("REGISTRY BEACON LOST // REGISTER ACTION FAILURE // claim released",6.0f);
        director.clearTerminalState();
        return;
    }

    if(spawns.empty()) return;

    if(sphericalSurfaceMode_) {
        const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
        const auto frame=planetSurface().surfaceFrame(feet);
        const Vec3 baseDir=normalize(feet);
        const float baseRadius=length(feet);
        for(const auto& request:spawns) {
            const Vec3 tangent=frame.right*std::cos(request.azimuthRadians)+frame.forward*std::sin(request.azimuthRadians);
            const Vec3 dir=normalize(baseDir+tangent*(request.distanceMeters/std::max(1.0f,baseRadius)));
            const Vec3 pos=dir*(surfaceRead().surfaceBoundaryRadius(dir)+1.8f);
            ecs_.queueSpawnEnemy(pos,enemyArchetypeFor(request.role),request.stableEnemyId,ActorDomain::PlanetSurface);
        }
    } else {
        const auto player=ecs_.player().position;
        for(const auto& request:spawns) {
            Vec3 pos=player+Vec3{std::sin(request.azimuthRadians)*request.distanceMeters,2.0f,
                                std::cos(request.azimuthRadians)*request.distanceMeters};
            pos.y=static_cast<float>(world().surfaceY(std::clamp(static_cast<int>(pos.x),1,World::Width-2),
                                                      std::clamp(static_cast<int>(pos.z),1,World::Depth-2)))+2.2f;
            ecs_.queueSpawnEnemy(pos,enemyArchetypeFor(request.role),request.stableEnemyId,ActorDomain::Planar);
        }
    }
    ecs_.flushCommands();

    const auto& state=director.state();
    if(state.type==ImperialEnforcementType::RegisterAction) {
        setMessage(std::string("REGISTER ACTION // WAVE ")+std::to_string(state.waveIndex+1)+"/"+
                   std::to_string(state.totalWaves)+" // "+imperialAttentionBandName(state.band),5.0f);
    } else setMessage("IMPERIAL PATROL // CONTACT",3.5f);
    (void)waveBefore;
}

bool Game::nearShip() const {
    if (sphericalSurfaceMode_) {
        const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
        return length(feet-sphericalShipTerminalPosition()) < 4.5f;
    }
    return distance2D(ecs_.player().position, shipTerminalPosition(world())) < 4.2f;
}

void Game::switchPlanet(int index) {
    if (index < 0 || index >= PlanetCount) return;
    currentPlanet_ = index;
    renderer_.invalidate();
    planetRenderer_.invalidate();
    orbitWalkerPosition_={};
    sphericalVerticalVelocity_=0.0f;
    sphericalGrounded_=true;
    ecs_.clearEnemies();
    ecs_.setPlayerPosition(spawnPoint(world()));
    if (sphericalSurfaceMode_) sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]=sphericalArrivalPosition();
    resetPlayerMetersAtShip();
    travelMenu_ = false;
    setMessage(std::string("ARRIVAL // ") + planetName(world().planetClass()), 4.0f);
}

void Game::resetPlayerMetersAtShip() {
    auto s = ecs_.savePlayer();
    s.health = std::max(75.0f, s.health);
    s.oxygen = 100.0f;
    s.energy = 100.0f;
    s.hunger = std::max(70.0f, s.hunger);
    ecs_.loadPlayer(s);
}

int Game::placedStructureCount(const World& w) const {
    int count = 0;
    for (const auto& [idx, type] : w.edits()) {
        const IVec3 c = w.cellFromFlatIndex(idx);
        if (type != BlockType::Air && type != w.baseline(c.x,c.y,c.z) && blockProperties(type).placeable) ++count;
    }
    return count;
}

float Game::claimFloor() const {
    int totalStructures = 0;
    bool anyClaim = false;
    for (int i = 0; i < PlanetCount; ++i) {
        if (!claimed_[static_cast<std::size_t>(i)]) continue;
        anyClaim = true;
        totalStructures += placedStructureCount(*worlds_[static_cast<std::size_t>(i)]);
        totalStructures += static_cast<int>(planetSurfaces_[static_cast<std::size_t>(i)]->placedMarkerCount());
        totalStructures += static_cast<int>(bases_[static_cast<std::size_t>(i)]->objects().size());
        totalStructures += static_cast<int>(surfaceBases_[static_cast<std::size_t>(i)]->objects().size());
    }
    if (!anyClaim) return 0.0f;
    return std::min(70.0f, 8.0f + 2.0f * std::sqrt(static_cast<float>(totalStructures) / 100.0f));
}

void Game::raiseSuspicion(float amount) {
    suspicion_ = std::clamp(suspicion_ + amount, claimFloor(), 100.0f);
}

void Game::setMessage(std::string text, float seconds) {
    message_ = std::move(text);
    messageTimer_ = seconds;
}

void Game::draw() {
    const auto env = world().environment();
    if (orbitalPreview_) {
        drawOrbitalPreview();
        return;
    }
    if (sphericalSurfaceMode_) {
        drawSphericalSurface();
        return;
    }
    const Vec3 cp = cameraPosition();
    const Vec3 cf = cameraForward();
    camera_.position = rv(cp);
    camera_.target = rv(cp + cf);
    camera_.up = {0,1,0};
    camera_.fovy = 72.0f;

    BeginDrawing();
    ClearBackground(skyColorForTime(env));
    BeginMode3D(camera_);
    drawCinematicAtmosphere(env, cp, Vec3{0,1,0});
    drawWorld3D();
    drawAmbientParticles(env, cp, Vec3{0,1,0});
    EndMode3D();
    drawHud();
    if (travelMenu_) drawTravelMenu();
    if (craftingMenu_) drawCraftingMenu();
    if (mapMenu_) drawMapMenu();
    EndDrawing();
}

void Game::drawOrbitalPreview() {
    const auto env=world().environment();
    planetRenderer_.setOrbitalShellOnly();
    planetRenderer_.sync(planetSurface());
    const float cp=std::cos(orbitPitch_);
    const Vec3 eye{std::sin(orbitYaw_)*cp*orbitDistance_,std::sin(orbitPitch_)*orbitDistance_,std::cos(orbitYaw_)*cp*orbitDistance_};
    camera_.position=rv(eye);
    camera_.target={0,0,0};
    camera_.up={0,1,0};
    camera_.fovy=58.0f;
    BeginDrawing();
    ClearBackground(Color{static_cast<unsigned char>(env.skyColor.r/4),static_cast<unsigned char>(env.skyColor.g/4),static_cast<unsigned char>(env.skyColor.b/4),255});
    BeginMode3D(camera_);
    planetRenderer_.drawOrbitalShell();
    if (lengthSq(orbitWalkerPosition_)>1.0f) {
        DrawSphere(rv(orbitWalkerPosition_),0.75f,Color{78,239,158,255});
        DrawSphere(rv(orbitWalkerPosition_),0.28f,RAYWHITE);
    }
    EndMode3D();
    drawOrbitalHud();
    EndDrawing();
}

void Game::drawSphericalSurface() {
    const auto env=world().environment();
    planetRenderer_.setStreamingFocus(sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)],12,16);
    planetRenderer_.sync(planetSurface());
    const Vec3 cp=sphericalCameraPosition();
    const Vec3 cf=sphericalCameraForward();
    const Vec3 up=normalize(sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)]);
    camera_.position=rv(cp);
    camera_.target=rv(cp+cf);
    camera_.up=rv(up);
    camera_.fovy=72.0f;

    BeginDrawing();
    ClearBackground(skyColorForTime(env));
    BeginMode3D(camera_);
    drawCinematicAtmosphere(env, cp, up);
    planetRenderer_.draw();
    drawSurfaceShipTerminal3D();
    drawSurfaceMachines3D();
    drawSurfacePortals3D();
    for (const auto& e:ecs_.enemies(ActorDomain::PlanetSurface)) {
        const Vec3 eup=lengthSq(e.position)>0.001f?normalize(e.position):Vec3{0,1,0};
        const float radius=enemyRenderRadius(e.archetype);
        const Vec3 center=e.position+eup*(radius+0.05f);
        DrawSphere(rv(center),radius,enemyRenderColor(e.archetype));
        DrawSphere(rv(center+eup*(radius*0.42f)),std::max(0.10f,radius*0.24f),enemyAccentColor(e.archetype));
        DrawSphereWires(rv(center),radius*1.08f,8,8,enemyAccentColor(e.archetype));
        DrawCylinderEx(rv(center-eup*radius*0.2f),rv(center+eup*radius*0.85f),radius*0.55f,radius*0.18f,8,Color{16,20,18,220});
    }
    drawAmbientParticles(env, cp, up);
    const auto hit=surfaceRead().raycast(cp,cf,7.0f,0.08f);
    if (hit.hit) drawSurfaceCellWires(planetSurface(),hit.cell,RAYWHITE);
    EndMode3D();
    drawSphericalHud();
    if (travelMenu_) drawTravelMenu();
    if (craftingMenu_) drawCraftingMenu();
    if (mapMenu_) drawMapMenu();
    EndDrawing();
}

void Game::drawSphericalHud() {
    const int sw=GetScreenWidth();
    const int sh=GetScreenHeight();
    const auto p=ecs_.player();
    const Vec3 feet=sphericalPlayerPositions_[static_cast<std::size_t>(currentPlanet_)];
    const auto cell=planetSurface().locate(feet);
    const auto& bp=blockProperties(selectedBlock_);
    const auto ps=surfaceBase().summary();

    // Soft vignette for cinematic presence.
    DrawRectangle(0,0,sw,48,Color{5,8,10,110});
    DrawRectangle(0,sh-70,sw,70,Color{5,8,10,120});

    DrawText("ELYSIUM",28,22,28,Color{114,245,176,255});
    DrawText(planetName(world().planetClass()),28,54,15,Color{183,208,196,255});

    if (!cinematicHud_) {
        DrawRectangle(16,78,640,220,Color{8,12,14,220});
        DrawRectangleLines(16,78,640,220,Color{72,165,131,255});
        DrawText(TextFormat("%s  U%02d V%02d R%02d // %s",toString(cell.face).c_str(),cell.u,cell.v,cell.radial,sphericalGrounded_?"GROUNDED":"RADIAL FALL"),28,90,16,Color{114,245,176,255});
        DrawText(TextFormat("LOD0 %d / near %d / far %d // %d quads // hostiles %d",planetRenderer_.fullDetailChunks(),planetRenderer_.nearFieldChunks(),planetRenderer_.farFieldChunks(),planetRenderer_.quads(),static_cast<int>(ecs_.enemies(ActorDomain::PlanetSurface).size())),28,114,14,GRAY);
        DrawText(TextFormat("power %.0f gen / %.0f demand / %.0f supplied // battery %.0f/%.0f",ps.generation,ps.demand,ps.supplied,ps.batteryStored,ps.batteryCapacity),28,136,14,Color{190,205,198,255});
        DrawText("F3 toggles cinematic HUD",28,160,13,GRAY);
    } else {
        DrawText(TextFormat("%s",sphericalGrounded_?"SURFACE LOCK":"BALLISTIC"),28,76,14,Color{114,245,176,200});
        DrawText("F3 SYSTEMS",28,96,12,Color{140,160,150,180});
    }

    drawBar(20,sh-126,225,22,p.health,"VIT");
    drawBar(20,sh-100,225,22,p.oxygen,"O2 ");
    drawBar(20,sh-74,225,22,p.energy,"ENG");
    drawBar(20,sh-48,225,22,p.hunger,"HNG");

    const int sx=sw-290;
    DrawRectangle(sx,18,270,78,Color{8,12,14,205});
    DrawRectangleLines(sx,18,270,78,Color{112,139,129,220});
    DrawText(TextFormat("SUSPICION  %05.1f",suspicion_),sx+12,28,17,RAYWHITE);
    DrawRectangle(sx+12,52,246,10,Color{31,35,39,255});
    DrawRectangle(sx+12,52,static_cast<int>(246.0f*suspicion_/100.0f),10,Color{164,93,85,255});
    DrawText(claimed_[static_cast<std::size_t>(currentPlanet_)]?"CLAIM: FILED":"CLAIM: UNFILED",sx+12,68,14,
             claimed_[static_cast<std::size_t>(currentPlanet_)]?Color{83,224,151,255}:Color{196,196,196,255});

    DrawRectangle(sw/2-330,sh-54,660,40,Color{8,12,14,220});
    DrawRectangleLines(sw/2-330,sh-54,660,40,Color{112,139,129,220});
    if (sculptMode_) DrawText("V MICRO SCULPT // LMB cuts one persistent 6.25 cm cell",sw/2-315,sh-43,15,Color{114,245,176,255});
    else if (portalMode_) DrawText(TextFormat("[B/0] PORTAL %s x%d // RMB place  E toggle",surfacePortalName(selectedPortal_),ecs_.inventoryCount(surfacePortalItemId(selectedPortal_))),sw/2-315,sh-43,15,Color{114,225,177,255});
    else if (machineMode_) DrawText(TextFormat("[/] MACHINE %s x%d // RMB place  E interact",machineName(selectedMachine_),ecs_.inventoryCount(machineItemId(selectedMachine_))),sw/2-315,sh-43,15,Color{210,170,140,255});
    else DrawText(TextFormat("[1-5] BUILD %s x%d // tool T%d",bp.name.data(),ecs_.inventoryCount(static_cast<int>(selectedBlock_)),toolTier_),sw/2-315,sh-43,16,RAYWHITE);

    DrawCircleLines(sw/2,sh/2,7,Color{255,255,255,180});
    DrawLine(sw/2-10,sh/2,sw/2-4,sh/2,RAYWHITE);
    DrawLine(sw/2+4,sh/2,sw/2+10,sh/2,RAYWHITE);
    DrawLine(sw/2,sh/2-10,sw/2,sh/2-4,RAYWHITE);
    DrawLine(sw/2,sh/2+4,sw/2,sh/2+10,RAYWHITE);

    if (messageTimer_ > 0.0f) {
        const int tw = MeasureText(message_.c_str(), 18);
        DrawRectangle(sw/2 - tw/2 - 16, 122, tw + 32, 34, Color{9,13,15,225});
        DrawRectangleLines(sw/2 - tw/2 - 16, 122, tw + 32, 34, Color{108,154,135,230});
        DrawText(message_.c_str(), sw/2 - tw/2, 131, 18, RAYWHITE);
    }

    if (nearShip()) DrawText("[T] SHIP NAVIGATION   [E] LIFE SUPPORT",sw/2-190,sh-92,17,Color{102,238,168,255});
}

void Game::drawWorld3D() {
    renderer_.sync(world());
    renderer_.draw();
    drawShipTerminal3D();
    drawMachines3D();

    for (const auto& e : ecs_.enemies(ActorDomain::Planar)) {
        const float scale=enemyRenderRadius(e.archetype)/0.42f;
        const Vector3 p=rv(e.position+Vec3{0,0.45f*scale,0});
        DrawCube(p,0.82f*scale,0.55f*scale,0.82f*scale,enemyRenderColor(e.archetype));
        DrawCubeWires(p,0.84f*scale,0.57f*scale,0.84f*scale,enemyAccentColor(e.archetype));
        DrawSphere(rv(e.position+Vec3{0,0.62f*scale,0}),0.11f*scale,enemyAccentColor(e.archetype));
    }

    if (sculptMode_) {
        const auto hit = world().raycastMicro(cameraPosition(), cameraForward(), 7.0f);
        if (hit) {
            constexpr float s = 1.0f / static_cast<float>(MicroBrick::Resolution);
            DrawCubeWires(Vector3{(hit->globalMicro.x + 0.5f)*s, (hit->globalMicro.y + 0.5f)*s, (hit->globalMicro.z + 0.5f)*s},
                          s*1.04f, s*1.04f, s*1.04f, Color{100,255,184,255});
        }
    } else {
        const auto hit = world().raycast(cameraPosition(), cameraForward(), 7.0f);
        if (hit) {
            DrawCubeWires(Vector3{hit->cell.x + 0.5f, hit->cell.y + 0.5f, hit->cell.z + 0.5f}, 1.01f,1.01f,1.01f, RAYWHITE);
        }
    }
}

void Game::drawShipTerminal3D() {
    const Vec3 t = shipTerminalPosition(world());
    DrawCube(rv(t + Vec3{0,-0.35f,0}), 5.0f, 0.3f, 4.0f, Color{54,61,66,255});
    DrawCubeWires(rv(t + Vec3{0,-0.35f,0}), 5.0f, 0.3f, 4.0f, Color{121,139,144,255});
    DrawCube(rv(t + Vec3{0,0.75f,0}), 0.8f, 1.6f, 0.8f, Color{16,33,29,255});
    DrawCubeWires(rv(t + Vec3{0,0.75f,0}), 0.84f, 1.64f, 0.84f, Color{52,218,144,255});
    DrawSphere(rv(t + Vec3{0,1.7f,0}), 0.18f, Color{72,236,163,255});
}


void Game::drawSurfaceShipTerminal3D() {
    const Vec3 t=sphericalShipTerminalPosition();
    const Vec3 up=::elysium::normalize(t);
    const auto frame=planetSurface().surfaceFrame(t);
    // Radially aligned proxy assembled from spheres/lines so no global-Y
    // assumption leaks into the spherical gameplay path.
    DrawSphere(rv(t),0.62f,Color{46,54,58,255});
    DrawSphere(rv(t+up*0.72f),0.20f,Color{21,41,35,255});
    DrawSphere(rv(t+up*0.98f),0.10f,Color{72,236,163,255});
    DrawLine3D(rv(t-frame.right*1.35f),rv(t+frame.right*1.35f),Color{105,124,128,255});
    DrawLine3D(rv(t-frame.forward*1.15f),rv(t+frame.forward*1.15f),Color{105,124,128,255});
}

void Game::drawSurfaceMachines3D() {
    for (const auto& o:surfaceBase().objects()) {
        const Vec3 center=planetSurface().cellCenterPosition(o.anchor);
        const Vec3 up=::elysium::normalize(center);
        Color body{76,82,84,255};
        Color accent=o.powered?Color{69,225,147,255}:Color{126,89,74,255};
        switch(o.type) {
            case MachineType::BurnerGenerator: body=Color{89,72,61,255}; break;
            case MachineType::BatteryBank: body=Color{69,78,91,255}; break;
            case MachineType::AtmosphereUnit: body=Color{58,89,87,255}; break;
            case MachineType::StorageCrate: body=Color{95,83,63,255}; break;
            case MachineType::AirlockController: body=Color{54,100,104,255}; break;
            case MachineType::SensorMast: body=Color{60,88,104,255}; break;
            case MachineType::Turret: body=Color{104,66,58,255}; break;
            case MachineType::ShieldPylon: body=Color{61,78,118,255}; break;
            case MachineType::LogicController: body=Color{73,96,78,255}; break;
            case MachineType::Furnace: body=Color{112,76,56,255}; break;
            case MachineType::AlloyCrucible: body=Color{103,72,90,255}; break;
            case MachineType::Refinery: body=Color{74,96,103,255}; break;
            case MachineType::NetworkStorage: body=Color{67,103,84,255}; break;
            case MachineType::Conveyor: body=Color{91,91,76,255}; break;
            case MachineType::Sorter: body=Color{78,93,108,255}; break;
            case MachineType::CargoLoader: body=Color{95,84,104,255}; break;
            case MachineType::Crusher: body=Color{105,91,66,255}; break;
            case MachineType::ChemicalVat: body=Color{77,112,82,255}; break;
            case MachineType::Fabricator: body=Color{74,88,117,255}; break;
            case MachineType::Extractor: body=Color{114,82,63,255}; break;
        }
        // Sphere-based proxy avoids assuming world Y is up; the radial marker
        // makes the machine's local orientation readable on every cube face.
        DrawSphere(rv(center),0.38f,body);
        DrawSphere(rv(center+up*0.38f),0.08f,accent);
        if(o.type==MachineType::BurnerGenerator && o.fuelSeconds>0.0f)
            DrawSphere(rv(center+up*0.52f),0.07f,Color{235,133,73,255});
    }
}

void Game::drawSurfacePortals3D() {
    for(const auto& portal:surfaceBase().portals()) {
        const Vec3 center=planetSurface().cellCenterPosition(portal.anchor);
        const Vec3 up=normalize(center);
        const auto frame=planetSurface().surfaceFrame(center);
        const Color edge=portal.type==SurfacePortalType::Airlock?Color{78,214,164,255}:Color{166,174,170,255};
        const Color state=portal.open?Color{230,143,88,255}:Color{71,223,151,255};
        if(!portal.open) DrawSphere(rv(center),0.30f,portal.type==SurfacePortalType::Airlock?Color{42,70,67,255}:Color{75,82,84,255});
        DrawLine3D(rv(center-frame.right*0.42f),rv(center+frame.right*0.42f),edge);
        DrawLine3D(rv(center-frame.forward*0.42f),rv(center+frame.forward*0.42f),edge);
        DrawSphere(rv(center+up*0.42f),0.07f,state);
    }
}

void Game::drawMachines3D() {
    for(const auto& o:base().objects()) {
        const Vector3 p{static_cast<float>(o.anchor.x)+0.5f,static_cast<float>(o.anchor.y)+0.5f,static_cast<float>(o.anchor.z)+0.5f};
        Color body{76,82,84,255};
        Color accent=o.powered?Color{69,225,147,255}:Color{126,89,74,255};
        switch(o.type) {
            case MachineType::BurnerGenerator: body=Color{89,72,61,255}; break;
            case MachineType::BatteryBank: body=Color{69,78,91,255}; break;
            case MachineType::AtmosphereUnit: body=Color{58,89,87,255}; break;
            case MachineType::StorageCrate: body=Color{95,83,63,255}; break;
            case MachineType::AirlockController: body=Color{54,100,104,255}; break;
            case MachineType::SensorMast: body=Color{60,88,104,255}; break;
            case MachineType::Turret: body=Color{104,66,58,255}; break;
            case MachineType::ShieldPylon: body=Color{61,78,118,255}; break;
            case MachineType::LogicController: body=Color{73,96,78,255}; break;
            case MachineType::Furnace: body=Color{112,76,56,255}; break;
            case MachineType::AlloyCrucible: body=Color{103,72,90,255}; break;
            case MachineType::Refinery: body=Color{74,96,103,255}; break;
            case MachineType::NetworkStorage: body=Color{67,103,84,255}; break;
            case MachineType::Conveyor: body=Color{91,91,76,255}; break;
            case MachineType::Sorter: body=Color{78,93,108,255}; break;
            case MachineType::CargoLoader: body=Color{95,84,104,255}; break;
            case MachineType::Crusher: body=Color{105,91,66,255}; break;
            case MachineType::ChemicalVat: body=Color{77,112,82,255}; break;
            case MachineType::Fabricator: body=Color{74,88,117,255}; break;
            case MachineType::Extractor: body=Color{114,82,63,255}; break;
        }
        DrawCube(p,0.72f,0.72f,0.72f,body);
        DrawCubeWires(p,0.75f,0.75f,0.75f,accent);
        if(o.type==MachineType::BurnerGenerator && o.fuelSeconds>0.0f)
            DrawSphere(Vector3{p.x,p.y+0.48f,p.z},0.08f,Color{235,133,73,255});
    }
}

void Game::drawOrbitalHud() {
    const int sw=GetScreenWidth();
    DrawRectangle(18,18,620,140,Color{5,9,12,225});
    DrawRectangleLines(18,18,620,140,Color{72,165,131,255});
    DrawText("CUBE-SPHERE ORBITAL DEBUG",34,30,24,RAYWHITE);
    DrawText(TextFormat("6 faces x %d x %d columns // %d radial layers",PlanetSurface::FaceResolution,PlanetSurface::FaceResolution,PlanetSurface::RadialLayers),34,62,16,Color{183,208,196,255});
    DrawText(TextFormat("ORBIT SHELL climate %d quads / clouds %d // voxel LOD0 not requested",planetRenderer_.orbitalClimateQuads(),planetRenderer_.orbitalCloudQuads()),34,86,16,RAYWHITE);
    const auto probe=planetSurface().locate(orbitWalkerPosition_);
    DrawText(TextFormat("probe %s U%02d V%02d R%02d // WASD walks tangent frame",toString(probe.face).c_str(),probe.u,probe.v,probe.radial),34,108,14,Color{114,245,176,255});
    DrawText("Mouse orbit  Wheel zoom  O/Esc return // P enters playable spherical surface",34,128,14,GRAY);
    DrawText(TextFormat("%s",planetName(world().planetClass())),sw-360,28,16,Color{114,245,176,255});
}

void Game::drawHud() {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    const auto p = ecs_.player();

    DrawRectangle(16, 16, 390, 72, Color{8,12,14,205});
    DrawRectangleLines(16, 16, 390, 72, Color{112,139,129,220});
    DrawText("ELYSIUM", 28, 22, 26, RAYWHITE);
    DrawText(planetName(world().planetClass()), 28, 52, 15, Color{173,196,185,255});

    drawBar(20, sh - 126, 225, 22, p.health, "VIT");
    drawBar(20, sh - 100, 225, 22, p.oxygen, "O2 ");
    drawBar(20, sh - 74, 225, 22, p.energy, "ENG");
    drawBar(20, sh - 48, 225, 22, p.hunger, "HNG");

    const int sx = sw - 290;
    DrawRectangle(sx, 18, 270, 98, Color{8,12,14,205});
    DrawRectangleLines(sx, 18, 270, 98, Color{112,139,129,220});
    DrawText(TextFormat("SYSTEM SUSPICION  %05.1f", suspicion_), sx + 12, 28, 17, RAYWHITE);
    DrawRectangle(sx + 12, 55, 246, 12, Color{31,35,39,255});
    DrawRectangle(sx + 12, 55, static_cast<int>(246.0f * suspicion_/100.0f), 12, Color{164,93,85,255});
    DrawText(TextFormat("Claim floor %.1f   Favor %.0f", claimFloor(), favor_), sx + 12, 74, 14, Color{190,198,194,255});
    DrawText(claimed_[static_cast<std::size_t>(currentPlanet_)] ? "CLAIM: FILED" : "CLAIM: UNFILED", sx + 12, 94, 14,
             claimed_[static_cast<std::size_t>(currentPlanet_)] ? Color{83,224,151,255} : Color{196,196,196,255});

    const auto& bp = blockProperties(selectedBlock_);
    DrawRectangle(sw/2 - 300, sh - 54, 600, 40, Color{8,12,14,210});
    DrawRectangleLines(sw/2 - 300, sh - 54, 600, 40, Color{112,139,129,220});
    if(machineMode_) DrawText(TextFormat("[6-9/K/J/U/Y/L/G/H/I/N] MACHINE: %s x%d  |  TOOL TIER %d  |  V: %s", machineName(selectedMachine_), ecs_.inventoryCount(machineItemId(selectedMachine_)), toolTier_, sculptMode_ ? "MICRO" : "MACRO"),
             sw/2 - 288, sh - 43, 16, RAYWHITE);
    else DrawText(TextFormat("[1-5] BUILD: %s x%d  |  TOOL TIER %d  |  V: %s", bp.name.data(), ecs_.inventoryCount(static_cast<int>(selectedBlock_)), toolTier_, sculptMode_ ? "MICRO" : "MACRO"),
             sw/2 - 288, sh - 43, 16, RAYWHITE);

    DrawLine(sw/2 - 8, sh/2, sw/2 + 8, sh/2, RAYWHITE);
    DrawLine(sw/2, sh/2 - 8, sw/2, sh/2 + 8, RAYWHITE);

    if (sculptMode_) {
        const auto hit = world().raycastMicro(cameraPosition(), cameraForward(), 7.0f);
        if (hit) DrawText(TextFormat("MICRO // %s  %d,%d,%d", blockProperties(hit->type).name.data(), hit->micro.x, hit->micro.y, hit->micro.z), sw/2 + 18, sh/2 + 16, 14, Color{114,245,176,255});
    } else {
        const auto hit = world().raycast(cameraPosition(), cameraForward(), 7.0f);
        if (hit) {
            DrawText(TextFormat("%s  T%d", blockProperties(hit->type).name.data(), blockProperties(hit->type).harvestTier), sw/2 + 18, sh/2 + 16, 14, RAYWHITE);
            if (miningProgress_ > 0.0f) {
                DrawRectangle(sw/2 - 80, sh/2 + 42, 160, 8, Color{25,25,25,220});
                DrawRectangle(sw/2 - 80, sh/2 + 42, static_cast<int>(160 * std::clamp(miningProgress_,0.0f,1.0f)), 8, RAYWHITE);
            }
        }
    }

    DrawText("LMB mine/chisel  RMB build/machine  1-9 hotbar  V micro  F attack  C craft  M chart  T travel  P spherical  O orbit", 20, sh - 154, 15, Color{222,226,223,230});
    DrawText(TextFormat("Greedy quads %d (macro %d / micro %d)  tris %d  mesh jobs %d/%zu  dirty %d rebuilt %d", renderer_.meshQuads(), renderer_.macroQuads(), renderer_.microQuads(), renderer_.triangleCount(), renderer_.pendingJobs(), jobs_.workerCount(),renderer_.dirtyChunks(),renderer_.rebuiltChunksLastSync()),
             20, 96, 14, Color{220,225,222,210});
    const auto ps=base().summary();
    DrawText(TextFormat("Base power %.0f gen / %.0f demand / %.0f supplied  battery %.0f/%.0f  machines %d",ps.generation,ps.demand,ps.supplied,ps.batteryStored,ps.batteryCapacity,static_cast<int>(base().objects().size())),
             20, 114, 14, Color{220,225,222,210});

    if (nearShip()) DrawText("[T] NAVIGATION   [E] SHIP LIFE SUPPORT", sw/2 - 175, sh - 92, 17, Color{102,238,168,255});
    else if (base().nearest(MachineType::BurnerGenerator,ecs_.player().position,2.8f)) DrawText("[E] FEED 1 COAL ORE TO BURNER GENERATOR",sw/2-205,sh-92,17,Color{235,160,100,255});

    if (messageTimer_ > 0.0f) {
        const int tw = MeasureText(message_.c_str(), 18);
        DrawRectangle(sw/2 - tw/2 - 16, 122, tw + 32, 34, Color{9,13,15,225});
        DrawRectangleLines(sw/2 - tw/2 - 16, 122, tw + 32, 34, Color{108,154,135,230});
        DrawText(message_.c_str(), sw/2 - tw/2, 131, 18, RAYWHITE);
    }
}

void Game::drawTravelMenu() {
    const int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawRectangle(0,0,sw,sh,Color{5,8,10,205});
    DrawRectangle(sw/2-360, sh/2-280, 720, 560, Color{14,18,20,245});
    DrawRectangleLines(sw/2-360, sh/2-280, 720, 560, Color{70,198,139,255});
    DrawText("SHIP NAVIGATION // EIGHT-WORLD FRONTIER", sw/2-300, sh/2-250, 22, RAYWHITE);
    for (int i = 0; i < PlanetCount; ++i) {
        const auto& w = *worlds_[static_cast<std::size_t>(i)];
        const Color c = i == currentPlanet_ ? Color{81,235,156,255} : Color{190,219,199,255};
        DrawText(TextFormat("[%d] %s", i+1, planetName(w.planetClass())), sw/2-300, sh/2-200 + i*36, 18, c);
    }
    DrawText("Press 1-8 to transfer. T / Esc closes.", sw/2-300, sh/2+200, 16, GRAY);
}

void Game::drawCraftingMenu() {
    const int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawRectangle(0,0,sw,sh,Color{5,8,10,205});
    DrawRectangle(sw/2-410, sh/2-355, 820, 710, Color{14,18,20,245});
    DrawRectangleLines(sw/2-410, sh/2-355, 820, 710, Color{127,145,138,255});
    DrawText("FIELD FABRICATION // EARLY INFRASTRUCTURE", sw/2-375, sh/2-268, 24, RAYWHITE);
    DrawText("[1] Bronze mining head = 3 Copper + 1 Tin", sw/2-375, sh/2-220, 17, RAYWHITE);
    DrawText("[2] Steel mining head  = 4 Iron + 2 Coal", sw/2-375, sh/2-188, 17, RAYWHITE);
    DrawText("[3] Steel Plate        = 2 Iron + 1 Coal", sw/2-375, sh/2-156, 17, RAYWHITE);
    DrawText("[4] Registry Beacon    = 4 Steel Plate + 2 Copper", sw/2-375, sh/2-124, 17, RAYWHITE);
    DrawText("[5] Burner Generator   = 2 Steel Plate + 2 Copper", sw/2-375, sh/2-76, 17, Color{215,191,167,255});
    DrawText("[6] Battery Bank       = 2 Steel Plate + 3 Copper", sw/2-375, sh/2-44, 17, Color{215,191,167,255});
    DrawText("[7] Atmosphere Unit    = 3 Steel Plate + 2 Copper", sw/2-375, sh/2-12, 17, Color{215,191,167,255});
    DrawText("[8] Storage Crate      = 1 Steel Plate + 4 Planks", sw/2-375, sh/2+20, 17, Color{215,191,167,255});
    DrawText("[9] Door kit           = 1 Steel Plate + 2 Planks", sw/2-375, sh/2+52, 17, Color{196,206,202,255});
    DrawText("[0] Airlock kit        = 3 Steel Plate + 1 Copper", sw/2-375, sh/2+84, 17, Color{114,225,177,255});
    DrawText("[K] Airlock Controller = 2 Steel Plate + 2 Copper", sw/2-375, sh/2+112, 16, Color{114,225,177,255});
    DrawText("[J] Sensor Mast        = 2 Steel Plate + 2 Copper", sw/2-375, sh/2+140, 16, Color{221,177,135,255});
    DrawText("[U] Turret             = 4 Steel Plate + 2 Copper", sw/2-375, sh/2+168, 16, Color{221,177,135,255});
    DrawText("[Y] Shield Pylon       = 6 Steel Plate + 3 Copper", sw/2-375, sh/2+196, 16, Color{152,180,231,255});
    DrawText("[L] Logic Controller   = 2 Steel Plate + 3 Copper", sw/2-375, sh/2+224, 16, Color{141,211,168,255});
    DrawText("[G] Furnace  [H] Alloy Crucible  [I] Refinery  [N] Network Storage",sw/2-375,sh/2+248,14,Color{226,183,143,255});
    DrawText("[Q] x2 Conveyor  [R] Sorter  [X] Cargo Loader  [Z] Crusher",sw/2-375,sh/2+270,14,Color{208,194,142,255});
    DrawText("[F1] Chemical Vat  [F2] Fabricator  [F3] Extractor",sw/2-375,sh/2+292,14,Color{159,205,183,255});
    DrawText(TextFormat("ORE // Coal %d  Copper %d  Tin %d  Iron %d", ecs_.inventoryCount(static_cast<int>(BlockType::CoalOre)), ecs_.inventoryCount(static_cast<int>(BlockType::CopperOre)), ecs_.inventoryCount(static_cast<int>(BlockType::TinOre)), ecs_.inventoryCount(static_cast<int>(BlockType::IronOre))), sw/2-375, sh/2+314, 14, Color{183,208,196,255});
    DrawText("C / Esc closes", sw/2-375, sh/2+336, 15, GRAY);
}

void Game::drawMapMenu() {
    const int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawRectangle(0,0,sw,sh,Color{3,6,8,218});
    DrawRectangle(sw/2-390, sh/2-280, 780, 560, Color{10,15,18,248});
    DrawRectangleLines(sw/2-390, sh/2-280, 780, 560, Color{72,165,131,255});
    DrawText("SYSTEM FILE // LOCAL CHART", sw/2-350, sh/2-245, 25, RAYWHITE);
    DrawText(TextFormat("System Suspicion %.1f / 100   Favor %.0f", suspicion_, favor_), sw/2-350, sh/2-200, 19, RAYWHITE);
    for (int i = 0; i < PlanetCount; ++i) {
        const auto& w = *worlds_[static_cast<std::size_t>(i)];
        DrawText(TextFormat("%d %-12s | sphere %d+%d | machines %d | claim %s",
                            i+1, w.environment().name.data(),
                            static_cast<int>(planetSurfaces_[static_cast<std::size_t>(i)]->macroEditCount()),
                            static_cast<int>(planetSurfaces_[static_cast<std::size_t>(i)]->microOverrideCount()),
                            static_cast<int>(surfaceBases_[static_cast<std::size_t>(i)]->objects().size()+surfaceBases_[static_cast<std::size_t>(i)]->portals().size()),
                            claimed_[static_cast<std::size_t>(i)] ? "FILED" : "-"),
                 sw/2-350, sh/2-150 + i*38, 16, i == currentPlanet_ ? Color{81,235,156,255} : RAYWHITE);
    }
    DrawText("Mining creates territorial attention. Claims preserve a Suspicion floor.", sw/2-350, sh/2+180, 17, GRAY);
    DrawText("M / Esc closes", sw/2-350, sh/2+220, 16, GRAY);
}

void Game::saveGame() {
    if (!IsWindowReady()) return;
    std::ostringstream out;

    const auto previousManifestForCleanup=readCheckedAtomicGeneration(kSavePath,true);
    const auto previousSurfaceRefs=previousManifestForCleanup.loaded
        ? extractSurfaceManifestChunkRefs(previousManifestForCleanup.data)
        : std::vector<SurfaceManifestChunkRef>{};
    std::vector<SurfaceManifestChunkRef> currentSurfaceRefs;
    const std::uint64_t nextSaveGeneration=saveGeneration_+1;

    const auto p = ecs_.savePlayer();
    // Save schema 8 binds every spherical sidecar to the same whole-save
    // generation as the global manifest. If a crash occurs before manifest
    // publication, the old manifest selects the retained previous chunk
    // generations instead of accidentally mixing campaign states.
    out << "ELYSIUM_SAVE 9 1\n";
    out << "save_generation " << nextSaveGeneration << "\n";
    out << "seed " << galaxySeed_ << "\n";
    out << "planet " << currentPlanet_ << "\n";
    out << std::setprecision(9);
    out << "state " << suspicion_ << ' ' << favor_ << ' ' << toolTier_;
    for (int i = 0; i < PlanetCount; ++i) out << ' ' << (claimed_[static_cast<std::size_t>(i)] ? 1 : 0);
    out << "\n";
    out << "player " << p.position.x << ' ' << p.position.y << ' ' << p.position.z << ' '
        << p.health << ' ' << p.oxygen << ' ' << p.energy << ' ' << p.hunger << "\n";
    out << "inventory " << p.inventory.size() << "\n";
    for (const auto& [id,count] : p.inventory) out << id << ' ' << count << "\n";

    for (int i = 0; i < PlanetCount; ++i) {
        const auto& w = *worlds_[static_cast<std::size_t>(i)];
        const auto& edits = w.edits();
        out << "world " << i << ' ' << edits.size() << "\n";
        std::vector<std::pair<int, BlockType>> ordered(edits.begin(), edits.end());
        std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
        for (const auto& [idx,type] : ordered) out << idx << ' ' << static_cast<int>(type) << "\n";
    }

    for (int i = 0; i < PlanetCount; ++i) {
        const auto& bricks = worlds_[static_cast<std::size_t>(i)]->microBricks();
        std::vector<int> cellIndices;
        cellIndices.reserve(bricks.size());
        for (const auto& [idx, brick] : bricks) {
            if (brick.overrideCount() > 0) cellIndices.push_back(idx);
        }
        std::sort(cellIndices.begin(), cellIndices.end());
        out << "micro_world " << i << ' ' << cellIndices.size() << "\n";
        for (const int idx : cellIndices) {
            const auto it = bricks.find(idx);
            if (it == bricks.end()) continue;
            const auto overrides = it->second.overrides();
            out << "cell " << idx << ' ' << overrides.size() << "\n";
            for (const auto& [microIndex,type] : overrides)
                out << microIndex << ' ' << static_cast<int>(type) << "\n";
        }
    }

    for (int i=0;i<PlanetCount;++i) {
        const auto& objects=bases_[static_cast<std::size_t>(i)]->objects();
        out << "objects " << i << ' ' << objects.size() << "\n";
        for(const auto& o:objects)
            out << "obj " << o.stableId << ' ' << static_cast<int>(o.type) << ' ' << o.anchor.x << ' ' << o.anchor.y << ' ' << o.anchor.z << ' ' << o.enabled << ' ' << o.fuelSeconds << ' ' << o.storedEnergy << "\n";
    }

    for (int i=0;i<PlanetCount;++i) {
        const Vec3 sp=sphericalPlayerPositions_[static_cast<std::size_t>(i)];
        out << "surface_player " << i << ' ' << sp.x << ' ' << sp.y << ' ' << sp.z << "\n";

        const auto& surface=*planetSurfaces_[static_cast<std::size_t>(i)];
        const auto& infrastructure=*surfaceBases_[static_cast<std::size_t>(i)];
        std::vector<PlanetChunkAddress> addresses;
        addresses.reserve(surface.journals().size()+infrastructure.objects().size());
        auto addAddress=[&](const PlanetChunkAddress& a) {
            const auto key=stableChunkKey(a);
            if(std::none_of(addresses.begin(),addresses.end(),[&](const auto& e){return stableChunkKey(e)==key;})) addresses.push_back(a);
        };
        for(const auto& [_,journal]:surface.journals()) if(!journal.empty()) addAddress(journal.address);
        for(const auto& object:infrastructure.objects()) addAddress(surface.chunkOf(object.anchor));
        for(const auto& portal:infrastructure.portals()) addAddress(surface.chunkOf(portal.anchor));
        std::sort(addresses.begin(),addresses.end(),[](const auto& a,const auto& b){return stableChunkKey(a)<stableChunkKey(b);});

        SurfaceChunkStore store(kSurfaceChunkSaveRoot,i,surface.seed(),kSurfaceGeneratorVersion,kSurfaceGeneratorFingerprint);
        for(const auto& address:addresses) {
            std::string error;
            const auto record=makeSurfaceChunkRecord(i,surface,infrastructure,address,nextSaveGeneration);
            if(!store.save(record,&error)) {
                setMessage("SAVE FAILED // surface chunk transaction: "+error,5.0f);
                return;
            }
        }

        out << "surface_manifest " << i << ' ' << addresses.size() << "\n";
        for(const auto& address:addresses) {
            out << "surface_chunk " << static_cast<int>(address.face) << ' ' << address.u << ' ' << address.v << ' ' << address.radial << "\n";
            currentSurfaceRefs.push_back({i,address});
        }
    }

    out << "END\n";
    if (!out.good()) { setMessage("SAVE FAILED // could not serialize global manifest",5.0f); return; }
    std::string manifestError;
    if (!writeCheckedAtomicGeneration(kSavePath,out.str(),&manifestError)) {
        setMessage("SAVE FAILED // manifest transaction: "+manifestError,5.0f);
        return;
    }
    saveGeneration_=nextSaveGeneration;

    std::string pruneError;
    if(!pruneSurfaceChunkStore(kSurfaceChunkSaveRoot,currentSurfaceRefs,previousSurfaceRefs,&pruneError)) {
        setMessage("SAVE COMMITTED // sidecar cleanup deferred: "+pruneError,5.0f);
        return;
    }
    setMessage("Save generation committed across manifest + spherical chunks.");
}

bool Game::loadGame() {
    const auto manifest=readCheckedAtomicGeneration(kSavePath,true);
    if (!manifest.loaded) return false;
    std::istringstream in(manifest.data);

    std::string tag;
    int saveVersion{}, generatorVersion{};
    if (!(in >> tag >> saveVersion >> generatorVersion) || tag != "ELYSIUM_SAVE") return false;
    if ((saveVersion < 1 || saveVersion > 9) || generatorVersion != 1) return false;

    std::uint64_t fileSaveGeneration{};
    if(saveVersion>=8) {
        if(!(in>>tag>>fileSaveGeneration) || tag!="save_generation") return false;
        if(fileSaveGeneration==0) return false;
    }

    std::uint64_t seed{};
    int planet{};
    float suspicion{}, favor{};
    int tool{};
    std::array<int, PlanetCount> claimFlags{};
    PlayerPersistentState p{};

    if (!(in >> tag >> seed) || tag != "seed") return false;
    if (!(in >> tag >> planet) || tag != "planet") return false;
    if (!(in >> tag >> suspicion >> favor >> tool) || tag != "state") return false;
    const int claimCount = saveVersion >= 9 ? PlanetCount : 3;
    for (int i = 0; i < claimCount; ++i) {
        if (!(in >> claimFlags[static_cast<std::size_t>(i)])) return false;
    }
    if (!(in >> tag >> p.position.x >> p.position.y >> p.position.z >> p.health >> p.oxygen >> p.energy >> p.hunger) || tag != "player") return false;

    std::size_t invCount{};
    if (!(in >> tag >> invCount) || tag != "inventory") return false;
    p.inventory.clear();
    for (std::size_t i = 0; i < invCount; ++i) {
        int id{}, count{};
        if (!(in >> id >> count)) return false;
        const bool legacyItem=id>=0 && id<3000;
        const bool industryItem=id>=static_cast<int>(IndustryItemId::CopperIngot) && id<=static_cast<int>(IndustryItemId::CompositePanel);
        if ((legacyItem || industryItem) && count > 0) p.inventory.emplace_back(id,count);
    }

    galaxySeed_ = seed;
    initWorlds();
    const int savedPlanets = saveVersion >= 9 ? PlanetCount : 3;
    for (int wi = 0; wi < savedPlanets; ++wi) {
        int fileWorld{};
        std::size_t editCount{};
        if (!(in >> tag >> fileWorld >> editCount) || tag != "world" || fileWorld < 0 || fileWorld >= PlanetCount) return false;
        for (std::size_t e = 0; e < editCount; ++e) {
            int idx{}, type{};
            if (!(in >> idx >> type)) return false;
            if (type >= 0 && type < kBlockTypeCount)
                worlds_[static_cast<std::size_t>(fileWorld)]->applySavedEdit(idx, static_cast<BlockType>(type));
        }
    }

    if (saveVersion >= 2) {
        for (int wi = 0; wi < savedPlanets; ++wi) {
            int fileWorld{};
            std::size_t brickCount{};
            if (!(in >> tag >> fileWorld >> brickCount) || tag != "micro_world" || fileWorld < 0 || fileWorld >= PlanetCount) return false;
            for (std::size_t b = 0; b < brickCount; ++b) {
                int flatCell{};
                std::size_t overrideCount{};
                if (!(in >> tag >> flatCell >> overrideCount) || tag != "cell") return false;
                for (std::size_t m = 0; m < overrideCount; ++m) {
                    int microIndex{}, type{};
                    if (!(in >> microIndex >> type)) return false;
                    if (microIndex >= 0 && microIndex < MicroBrick::CellCount && type >= 0 && type < kBlockTypeCount)
                        worlds_[static_cast<std::size_t>(fileWorld)]->applySavedMicroEdit(flatCell,microIndex,static_cast<BlockType>(type));
                }
            }
        }
    }

    if (saveVersion >= 3) {
        for(int wi=0;wi<PlanetCount;++wi) {
            int fileWorld{}; std::size_t objectCount{};
            if(!(in>>tag>>fileWorld>>objectCount) || tag!="objects" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for(std::size_t oi=0;oi<objectCount;++oi) {
                MachineObject o{}; int type{}; int enabled{};
                if(!(in>>tag>>o.stableId>>type>>o.anchor.x>>o.anchor.y>>o.anchor.z>>enabled>>o.fuelSeconds>>o.storedEnergy) || tag!="obj") return false;
                if(type<0 || type>static_cast<int>(MachineType::ArcSmelter)) return false;
                o.type=static_cast<MachineType>(type); o.enabled=enabled!=0;
                if(!bases_[static_cast<std::size_t>(fileWorld)]->restore(o)) return false;
            }
            bases_[static_cast<std::size_t>(fileWorld)]->update(0.0f);
        }
    }

    if (saveVersion >= 4 && saveVersion <= 5) {
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{};
            Vec3 pos{};
            if (!(in>>tag>>fileWorld>>pos.x>>pos.y>>pos.z) || tag!="surface_player" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z) || lengthSq(pos)<1.0f) return false;
            sphericalPlayerPositions_[static_cast<std::size_t>(fileWorld)]=pos;

            std::size_t editCount{};
            if (!(in>>tag>>fileWorld>>editCount) || tag!="surface_world" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for (std::size_t e=0;e<editCount;++e) {
                int idx{},type{};
                if (!(in>>idx>>type)) return false;
                if (type>=0 && type<kBlockTypeCount)
                    planetSurfaces_[static_cast<std::size_t>(fileWorld)]->applySavedEdit(idx,static_cast<BlockType>(type));
            }

            std::size_t placedCount{};
            if (!(in>>tag>>fileWorld>>placedCount) || tag!="surface_placed" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for (std::size_t m=0;m<placedCount;++m) {
                int idx{};
                if (!(in>>idx)) return false;
                planetSurfaces_[static_cast<std::size_t>(fileWorld)]->applySavedPlacedMarker(idx);
            }
        }
    }

    if (saveVersion == 5) {
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{}; std::size_t brickCount{};
            if (!(in>>tag>>fileWorld>>brickCount) || tag!="surface_micro" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for (std::size_t b=0;b<brickCount;++b) {
                int flatCell{}; std::size_t overrideCount{};
                if (!(in>>tag>>flatCell>>overrideCount) || tag!="surface_cell") return false;
                for (std::size_t m=0;m<overrideCount;++m) {
                    int microIndex{},type{};
                    if (!(in>>microIndex>>type)) return false;
                    if (microIndex>=0 && microIndex<MicroBrick::CellCount && type>=0 && type<kBlockTypeCount)
                        planetSurfaces_[static_cast<std::size_t>(fileWorld)]->applySavedMicroEdit(flatCell,microIndex,static_cast<BlockType>(type));
                }
            }
        }
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{}; std::size_t objectCount{};
            if (!(in>>tag>>fileWorld>>objectCount) || tag!="surface_objects" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for (std::size_t oi=0;oi<objectCount;++oi) {
                SurfaceMachineObject o{}; int type{},face{},enabled{};
                if (!(in>>tag>>o.stableId>>type>>face>>o.anchor.u>>o.anchor.v>>o.anchor.radial>>enabled>>o.fuelSeconds>>o.storedEnergy) || tag!="surface_obj") return false;
                if (type<0 || type>static_cast<int>(MachineType::ArcSmelter) || face<0 || face>=PlanetSurface::FaceCount) return false;
                o.type=static_cast<MachineType>(type); o.anchor.face=static_cast<CubeFace>(face); o.enabled=enabled!=0;
                o.anchor=planetSurfaces_[static_cast<std::size_t>(fileWorld)]->normalize(o.anchor);
                if (!planetSurfaces_[static_cast<std::size_t>(fileWorld)]->radialInBounds(o.anchor.radial)) return false;
                if (!surfaceBases_[static_cast<std::size_t>(fileWorld)]->restore(o)) return false;
            }
            surfaceBases_[static_cast<std::size_t>(fileWorld)]->update(*planetSurfaces_[static_cast<std::size_t>(fileWorld)],0.0f);
        }
    }

    if (saveVersion == 6) {
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{};
            Vec3 pos{};
            if (!(in>>tag>>fileWorld>>pos.x>>pos.y>>pos.z) || tag!="surface_player" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z) || lengthSq(pos)<1.0f) return false;
            sphericalPlayerPositions_[static_cast<std::size_t>(fileWorld)]=pos;

            std::size_t journalCount{};
            if (!(in>>tag>>fileWorld>>journalCount) || tag!="surface_journals" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            auto& surface=*planetSurfaces_[static_cast<std::size_t>(fileWorld)];
            for (std::size_t ji=0;ji<journalCount;++ji) {
                int face{},cu{},cv{},cr{};
                std::size_t macroCount{},placedCount{},microCellCount{};
                if (!(in>>tag>>face>>cu>>cv>>cr>>macroCount>>placedCount>>microCellCount) || tag!="journal") return false;
                if (face<0 || face>=PlanetSurface::FaceCount) return false;
                const PlanetChunkAddress expected{static_cast<CubeFace>(face),cu,cv,cr};
                for (std::size_t e=0;e<macroCount;++e) {
                    int idx{},type{};
                    if (!(in>>tag>>idx>>type) || tag!="macro") return false;
                    if (idx<0 || idx>=PlanetSurface::TotalCells || type<0 || type>=kBlockTypeCount) return false;
                    if (!(surface.chunkOf(surface.cellFromFlatIndex(idx))==expected)) return false;
                    surface.applySavedEdit(idx,static_cast<BlockType>(type));
                }
                for (std::size_t m=0;m<placedCount;++m) {
                    int idx{};
                    if (!(in>>tag>>idx) || tag!="placed") return false;
                    if (idx<0 || idx>=PlanetSurface::TotalCells || !(surface.chunkOf(surface.cellFromFlatIndex(idx))==expected)) return false;
                    surface.applySavedPlacedMarker(idx);
                }
                for (std::size_t mc=0;mc<microCellCount;++mc) {
                    int idx{}; std::size_t overrideCount{};
                    if (!(in>>tag>>idx>>overrideCount) || tag!="micro") return false;
                    if (idx<0 || idx>=PlanetSurface::TotalCells || !(surface.chunkOf(surface.cellFromFlatIndex(idx))==expected)) return false;
                    for (std::size_t oi=0;oi<overrideCount;++oi) {
                        int microIndex{},type{};
                        if (!(in>>tag>>microIndex>>type) || tag!="microedit") return false;
                        if (microIndex<0 || microIndex>=MicroBrick::CellCount || type<0 || type>=kBlockTypeCount) return false;
                        surface.applySavedMicroEdit(idx,microIndex,static_cast<BlockType>(type));
                    }
                }
            }
        }
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{}; std::size_t objectCount{};
            if (!(in>>tag>>fileWorld>>objectCount) || tag!="surface_objects" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            for (std::size_t oi=0;oi<objectCount;++oi) {
                SurfaceMachineObject o{}; int type{},face{},enabled{};
                if (!(in>>tag>>o.stableId>>type>>face>>o.anchor.u>>o.anchor.v>>o.anchor.radial>>enabled>>o.fuelSeconds>>o.storedEnergy) || tag!="surface_obj") return false;
                if (type<0 || type>static_cast<int>(MachineType::ArcSmelter) || face<0 || face>=PlanetSurface::FaceCount) return false;
                o.type=static_cast<MachineType>(type); o.anchor.face=static_cast<CubeFace>(face); o.enabled=enabled!=0;
                o.anchor=planetSurfaces_[static_cast<std::size_t>(fileWorld)]->normalize(o.anchor);
                if (!planetSurfaces_[static_cast<std::size_t>(fileWorld)]->radialInBounds(o.anchor.radial)) return false;
                if (!surfaceBases_[static_cast<std::size_t>(fileWorld)]->restore(o)) return false;
            }
            surfaceBases_[static_cast<std::size_t>(fileWorld)]->update(*planetSurfaces_[static_cast<std::size_t>(fileWorld)],0.0f);
        }
    }

    if (saveVersion >= 7) {
        for (int wi=0;wi<savedPlanets;++wi) {
            int fileWorld{};
            Vec3 pos{};
            if (!(in>>tag>>fileWorld>>pos.x>>pos.y>>pos.z) || tag!="surface_player" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z) || lengthSq(pos)<1.0f) return false;
            sphericalPlayerPositions_[static_cast<std::size_t>(fileWorld)]=pos;

            std::size_t recordCount{};
            if (!(in>>tag>>fileWorld>>recordCount) || tag!="surface_manifest" || fileWorld<0 || fileWorld>=PlanetCount) return false;
            auto& surface=*planetSurfaces_[static_cast<std::size_t>(fileWorld)];
            auto& infrastructure=*surfaceBases_[static_cast<std::size_t>(fileWorld)];
            SurfaceChunkStore store(kSurfaceChunkSaveRoot,fileWorld,surface.seed(),kSurfaceGeneratorVersion,kSurfaceGeneratorFingerprint);
            for(std::size_t ri=0;ri<recordCount;++ri) {
                int face{},cu{},cv{},cr{};
                if(!(in>>tag>>face>>cu>>cv>>cr) || tag!="surface_chunk" || face<0 || face>=PlanetSurface::FaceCount) return false;
                const PlanetChunkAddress address{static_cast<CubeFace>(face),cu,cv,cr};
                const auto loaded=saveVersion>=8
                    ? store.load(address,fileSaveGeneration)
                    : store.load(address,std::uint64_t{0});
                if(!loaded.loaded) return false;
                const auto& record=loaded.record;
                for(const auto& [idx,type]:record.journal.macroEdits) surface.applySavedEdit(idx,type);
                for(const int idx:record.journal.placedMarkers) surface.applySavedPlacedMarker(idx);
                for(const auto& [idx,brick]:record.journal.microBricks)
                    for(const auto& [microIndex,type]:brick.overrides())
                        surface.applySavedMicroEdit(idx,static_cast<int>(microIndex),type);
                for(const auto& object:record.machines) if(!infrastructure.restore(object)) return false;
                for(const auto& portal:record.portals) if(!infrastructure.restorePortal(portal)) return false;
                for(const auto& airlock:record.airlocks) if(!infrastructure.restoreAirlockAssembly(airlock)) return false;
                for(const auto& rule:record.automationRules) if(!infrastructure.restoreAutomationRule(rule)) return false;
            }
            infrastructure.update(surface,0.0f);
        }
    }

    if (!(in >> tag) || tag != "END") return false;

    currentPlanet_ = std::clamp(planet, 0, PlanetCount-1);
    saveGeneration_=saveVersion>=8 ? fileSaveGeneration : 0;
    suspicion_ = std::clamp(suspicion, 0.0f, 100.0f);
    favor_ = std::clamp(favor, 0.0f, 100.0f);
    toolTier_ = std::clamp(tool, 1, 5);
    claimed_.fill(false);
    for (int i = 0; i < PlanetCount; ++i)
        claimed_[static_cast<std::size_t>(i)] = claimFlags[static_cast<std::size_t>(i)] != 0;
    ecs_.loadPlayer(p);
    renderer_.invalidate();
    planetRenderer_.invalidate();
    orbitWalkerPosition_={};
    sphericalSurfaceMode_=true;
    orbitalPreview_=false;
    sphericalVerticalVelocity_=0.0f;
    sphericalGrounded_=true;
    sphericalMiningValid_=false;
    ecs_.clearEnemies();
    return true;
}

float Game::dayFactor() const {
    const float angle = worldTimeOfDay_ * 6.28318530718f - 1.57079632679f;
    return std::clamp(std::sin(angle) * 0.85f + 0.25f, 0.05f, 1.0f);
}

Color Game::skyColorForTime(const PlanetEnvironment& env) const {
    const float day = dayFactor();
    const Color daySky = rc(env.skyColor);
    const Color nightSky{
        static_cast<unsigned char>(env.skyColor.r / 5),
        static_cast<unsigned char>(env.skyColor.g / 5),
        static_cast<unsigned char>(std::max(18, env.skyColor.b / 3)),
        255
    };
    return Color{
        static_cast<unsigned char>(nightSky.r + (daySky.r - nightSky.r) * day),
        static_cast<unsigned char>(nightSky.g + (daySky.g - nightSky.g) * day),
        static_cast<unsigned char>(nightSky.b + (daySky.b - nightSky.b) * day),
        255
    };
}

void Game::drawTitleScreen() {
    const int sw = GetScreenWidth();
    const int sh = GetScreenHeight();
    const auto env = world().environment();
    const float t = titleTimer_;

    planetRenderer_.setOrbitalShellOnly();
    planetRenderer_.sync(planetSurface());
    orbitYaw_ = 0.55f + t * 0.08f;
    orbitPitch_ = 0.28f + 0.05f * std::sin(t * 0.35f);
    orbitDistance_ = 88.0f;
    const float cp = std::cos(orbitPitch_);
    const Vec3 eye{std::sin(orbitYaw_)*cp*orbitDistance_, std::sin(orbitPitch_)*orbitDistance_, std::cos(orbitYaw_)*cp*orbitDistance_};
    camera_.position = rv(eye);
    camera_.target = {0,0,0};
    camera_.up = {0,1,0};
    camera_.fovy = 52.0f;

    BeginDrawing();
    ClearBackground(Color{5,8,12,255});
    BeginMode3D(camera_);
    planetRenderer_.drawOrbitalShell();
    EndMode3D();

    DrawRectangle(0, 0, sw, sh, Color{4, 8, 12, 120});
    DrawRectangleGradientV(0, 0, sw, sh/2, Color{10, 30, 34, 90}, Color{0,0,0,0});
    DrawRectangleGradientV(0, sh/2, sw, sh/2, Color{0,0,0,0}, Color{5,8,12,180});

    const char* brand = "ELYSIUM";
    const int brandSize = 72;
    const int bw = MeasureText(brand, brandSize);
    DrawText(brand, sw/2 - bw/2, sh/2 - 110, brandSize, Color{114,245,176,255});
    DrawText("THE SLEEPING EMPIRE", sw/2 - MeasureText("THE SLEEPING EMPIRE", 18)/2, sh/2 - 150, 18, Color{183,208,196,220});
    DrawText("Cube-sphere frontiers. Imperial suspicion. Survive the filed claim.",
             sw/2 - MeasureText("Cube-sphere frontiers. Imperial suspicion. Survive the filed claim.", 18)/2,
             sh/2 - 20, 18, Color{190,205,198,230});
    const float pulse = 0.55f + 0.45f * (0.5f + 0.5f * std::sin(t * 3.0f));
    DrawText("PRESS ENTER / SPACE TO DESCEND",
             sw/2 - MeasureText("PRESS ENTER / SPACE TO DESCEND", 20)/2,
             sh/2 + 40, 20, Color{114, 245, 176, static_cast<unsigned char>(80 + 175 * pulse)});
    DrawText(planetName(world().planetClass()),
             sw/2 - MeasureText(planetName(world().planetClass()), 16)/2, sh - 48, 16, Color{140,160,150,200});
    EndDrawing();
}

void Game::drawCinematicAtmosphere(const PlanetEnvironment& env, Vec3 cameraPos, Vec3 upHint) {
    const Vec3 up = lengthSq(upHint) > 0.001f ? normalize(upHint) : Vec3{0,1,0};
    const float day = dayFactor();
    const float angle = worldTimeOfDay_ * 6.28318530718f - 1.57079632679f;
    Vec3 sunDir{std::cos(angle), std::sin(angle) * 0.85f + 0.15f, std::sin(angle) * 0.35f};
    // Align sun into local tangent frame using radial up.
    Vec3 tangent = normalize(cross(up, Vec3{0,1,0}));
    if (lengthSq(tangent) < 0.1f) tangent = normalize(cross(up, Vec3{1,0,0}));
    Vec3 bitangent = normalize(cross(up, tangent));
    const Vec3 worldSun = normalize(tangent * sunDir.x + up * sunDir.y + bitangent * sunDir.z);
    const Vec3 sunPos = cameraPos + worldSun * 180.0f;
    const float sunScale = 3.2f + day * 2.5f;
    DrawSphere(rv(sunPos), sunScale, Color{255, 230, 170, static_cast<unsigned char>(120 + 100 * day)});
    DrawSphere(rv(sunPos), sunScale * 0.45f, Color{255, 248, 220, 255});

    // Horizon haze shells.
    const Color haze{
        env.horizonColor.r,
        env.horizonColor.g,
        env.horizonColor.b,
        static_cast<unsigned char>(18 + 28 * day)
    };
    DrawSphere(rv(cameraPos - up * 8.0f), 70.0f, haze);
    DrawSphereWires(rv(cameraPos), 220.0f, 16, 16, Color{env.horizonColor.r, env.horizonColor.g, env.horizonColor.b, 18});
}

void Game::drawAmbientParticles(const PlanetEnvironment& env, Vec3 focus, Vec3 upHint) {
    const Vec3 up = lengthSq(upHint) > 0.001f ? normalize(upHint) : Vec3{0,1,0};
    Vec3 tangent = normalize(cross(up, Vec3{0,1,0}));
    if (lengthSq(tangent) < 0.1f) tangent = normalize(cross(up, Vec3{1,0,0}));
    Vec3 bitangent = normalize(cross(up, tangent));
    const int count = 48;
    for (int i = 0; i < count; ++i) {
        const float phase = worldTimeOfDay_ * 40.0f + static_cast<float>(i) * 1.7f;
        const float ox = std::sin(phase * 0.7f + i) * 18.0f;
        const float oy = 1.5f + std::fmod(phase * 0.35f + i * 0.13f, 12.0f);
        const float oz = std::cos(phase * 0.55f + i * 0.4f) * 18.0f;
        const Vec3 p = focus + tangent * ox + up * oy + bitangent * oz;
        const unsigned char a = static_cast<unsigned char>(70 + (i % 5) * 20);
        DrawSphere(rv(p), 0.05f + (i % 3) * 0.02f, Color{env.horizonColor.r, env.horizonColor.g, env.horizonColor.b, a});
    }
}

} // namespace elysium
