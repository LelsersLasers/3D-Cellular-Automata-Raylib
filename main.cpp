#include "raylib.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <thread>
#include <iostream>
#include <fstream>

#include "json.hpp"

using std::string;
using std::vector;
using std::thread;

using json = nlohmann::json;


#define PI 3.14159265358979323846f

#define JSON_FILE "options.json"


enum NeighborType {
    MOORE,
    VON_NEUMANN
};

enum DrawMode {
    DUAL_COLOR = 0,
    RGB_CUBE = 1,
    DUAL_COLOR_DYING = 2,
    SINGLE_COLOR = 3,
    CENTER_DIST = 4
};

enum TickMode {
    FAST = 0,
    DYNAMIC = 1,
    MANUAL = 2
};


struct Vector3Int {
    int x, y, z;
};

bool SURVIVAL[27];
bool SPAWN[27];
int STATE;
NeighborType NEIGHBORHOODS;

Color dualColorAlive;
Color dualColorDead;
Vector3 colorOffset;
Color dualColorDyingAlive;
Color singleColorAlive;
Color centerDistMax;

int cellBounds;
size_t totalCells;
float aliveChanceOnSpawn;
size_t threads;
int targetFPS;


class ToggleKey {
private:
    bool wasDown = false;
public:
    bool down(bool pressState) {
        if (!wasDown && pressState) {
            wasDown = true;
            return true;
        }
        else if (!pressState) wasDown = false;
        return false;
    } 
    
};


class DrawableText {
private:
    string text;
public:
    DrawableText(string text) : text(text) {}
    void draw(int i) const {
        int x = 30;
        Color color = DARKGRAY;
        if (text[0] != '-') {
            x = 20;
            color = BLACK;
        }
        DrawText(text.c_str(), x, (i + 1) * 14, 10, color);
    }
    int length() const {
        return text.length();
    }
};


float calc_distance(Vector3Int a, Vector3Int b) {
    return sqrt(pow((float)a.x - b.x, 2) + pow((float)a.y - b.y, 2) + pow((float)a.z - b.z, 2));
}


class Cell {
private:
    Vector3 pos;
    Vector3Int index;
    int hp = -1;
    size_t neighbors = 0;
    static int aliveCells;
    static int deadCells;

    void draw(Color color) const { DrawCube(pos, 1.0f, 1.0f, 1.0f, color); }

public:
    Cell(Vector3Int index) {
        this->index = index;
        pos = {
            index.x - (cellBounds - 1.0f) / 2,
            index.y - (cellBounds - 1.0f) / 2,
            index.z - (cellBounds - 1.0f) / 2
        };
    }

    static void clearCellCounts() {
        aliveCells = 1;
        deadCells = 1;
    }
    static int getAliveCells() { return aliveCells; }
    static int getDeadCells() { return deadCells; }

    void clearNeighbors() { neighbors = 0; }
    void addNeighbor(int neighborAlive) { neighbors += neighborAlive; }
    int getHp() const { return hp; }
    void setHp(int hp) { this->hp = hp; }
    bool getAlive() const { return hp == STATE; }
    void reset() {
        hp = -1;
        neighbors = 0;
    }
    void randomizeState() {
        hp = ((double)rand() / (double)RAND_MAX < aliveChanceOnSpawn) * (STATE + 1) - 1;
    }
    void sync() {
        // Branchless by using bool -> int conversion
        hp = 
            (hp == STATE) * (hp - 1 + SURVIVAL[neighbors]) + // alive
            (hp < 0) * (SPAWN[neighbors] * (STATE + 1) - 1) +  // dead
            (hp >= 0 && hp < STATE) * (hp - 1); // dying

        aliveCells += hp == STATE;
        deadCells += hp < 0;
    }
    void jsonStateUpdate(int oldState) {
        hp = 
            (hp < 0) * -1 + // stay dead
            (hp >= 0) * (hp * (float)STATE/oldState); // scale hp
    }
    void drawDualColor() const {
        if (hp >= 0) {
            draw((Color){
                (unsigned char)(dualColorDead.r + colorOffset.x/(STATE + 1) * (hp + 1)),
                (unsigned char)(dualColorDead.g + colorOffset.y/(STATE + 1) * (hp + 1)),
                (unsigned char)(dualColorDead.b + colorOffset.z/(STATE + 1) * (hp + 1)),
                255
            });
        }
    }
    void drawRGBCube() const {
        if (hp >= 0) {
            draw((Color){
                (unsigned char)((float)index.x/cellBounds * 255),
                (unsigned char)((float)index.y/cellBounds * 255),
                (unsigned char)((float)index.z/cellBounds * 255),
                255
            });
        }
    }
    void drawDualColorDying() const {
        if (hp >= 0) {
            Color color = dualColorDyingAlive;
            if (hp < STATE) {
                float intensity = (1.0f + hp)/(STATE + 2.0f);
                unsigned char brightness = (int)(intensity * 255);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            draw(color);
        }
    }
    void drawSingleColor() const {
        if (hp >= 0) {
            float intensity = 3.0f/(STATE + 3.0f) + hp/(STATE + 3.0f);
            draw((Color){
                (unsigned char)(intensity * singleColorAlive.r),
                (unsigned char)(intensity * singleColorAlive.g),
                (unsigned char)(intensity * singleColorAlive.b),
                255
            });
        }
    }
    void drawDist() const {
        if (hp >= 0) {
            int cap = cellBounds/2;
            float dist = calc_distance(index, { cap, cap, cap });
            float intensity = 2.0f/(cap * sqrt(3.0f) + 2.0f) + dist/(cap * sqrt(3.0f) + 2.0f);
            draw((Color){
                (unsigned char)(intensity * centerDistMax.r),
                (unsigned char)(intensity * centerDistMax.g),
                (unsigned char)(intensity * centerDistMax.b),
                255
            });
        }
    }
};
int Cell::aliveCells = 1;
int Cell::deadCells = 1;


string textFromEnum(NeighborType nt) {
    switch (nt) {
        case MOORE: return "Moore";
        case VON_NEUMANN: return "von Neumann";
    }
    return "";
}
string textFromEnum(DrawMode dm) {
    switch (dm) {
        case DUAL_COLOR: return "Dual Color";
        case RGB_CUBE: return "RGB";
        case DUAL_COLOR_DYING: return "Dual Color Dying";
        case SINGLE_COLOR: return "Single Color";
        case CENTER_DIST: return "Center Dist";
    }
    return "";
}
string textFromEnum(TickMode tm) {
    switch (tm) {
        case MANUAL: return "Manual";
        case FAST: return "Fastest";
        case DYNAMIC: return "Dynamic";
    }
    return "";
}

void loadFromJSON() {
    std::cout << "Loading from JSON..." << std::endl;
    try {
        json rules;
        std::ifstream reader(JSON_FILE);
        reader >> rules;
        reader.close();

        for (size_t i = 0; i < 27; i++) SURVIVAL[i] = false;
        for (size_t i = 0; i < 27; i++) SPAWN[i] = false;

        for (size_t value : rules["survival"]) SURVIVAL[value] = true;
        for (size_t value : rules["spawn"]) SPAWN[value] = true;

        STATE = rules["state"];
        if (rules["neighborhood"] == "VN") NEIGHBORHOODS = VON_NEUMANN;
        else NEIGHBORHOODS = MOORE;

        dualColorAlive = {
            rules["dualColorAlive"][0],
            rules["dualColorAlive"][1],
            rules["dualColorAlive"][2],
            255
        };
        dualColorDead = {
            rules["dualColorDead"][0],
            rules["dualColorDead"][1],
            rules["dualColorDead"][2],
            255
        };
        colorOffset = {
            (float)(dualColorAlive.r - dualColorDead.r),
            (float)(dualColorAlive.g - dualColorDead.g),
            (float)(dualColorAlive.b - dualColorDead.b)
        };
        dualColorDyingAlive = {
            rules["dualColorDyingAlive"][0],
            rules["dualColorDyingAlive"][1],
            rules["dualColorDyingAlive"][2],
            255
        };
        singleColorAlive = {
            rules["singleColorAlive"][0],
            rules["singleColorAlive"][1],
            rules["singleColorAlive"][2],
            255
        };
        centerDistMax = {
            rules["centerDistMax"][0],
            rules["centerDistMax"][1],
            rules["centerDistMax"][2],
            255
        };
        
        cellBounds = rules["cellBounds"];
        totalCells = cellBounds * cellBounds * cellBounds;
        aliveChanceOnSpawn = rules["aliveChanceOnSpawn"];
        threads = rules["threads"];
        targetFPS = rules["targetFPS"];

        std::cout << "Finished loading from JSON..." << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << "JSON '" << JSON_FILE << "' not found or invalid." << std::endl;
        std::cout << "Exiting..." << std::endl;
        exit(EXIT_FAILURE);
    }
}

float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

size_t threeToOne(int x, int y, int z) {
    return x * cellBounds * cellBounds + y * cellBounds  + z;
}


bool validCellIndex(int x, int y, int z, const Vector3Int &offset) {
    return x + offset.x >= 0 && x + offset.x < cellBounds &&
           y + offset.y >= 0 && y + offset.y < cellBounds &&
           z + offset.z >= 0 && z + offset.z < cellBounds;
}

void syncCells(vector<Cell> &cells, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        cells[i].sync();
    }
}

void updateNeighbors(vector<Cell> &cells, int start, int end, const Vector3Int offsets[], size_t totalOffsets) {
    for (int x = start; x < end; x++) {
        for (int y = 0; y < cellBounds; y++) {
            for (int z = 0; z < cellBounds; z++) {
                int oneIdx = threeToOne(x, y, z);
                cells[oneIdx].clearNeighbors();
                for (size_t i = 0; i < totalOffsets; i++) {
                    if (validCellIndex(x, y, z, offsets[i])) {
                        cells[oneIdx].addNeighbor(cells[threeToOne(x + offsets[i].x, y + offsets[i].y, z + offsets[i].z)].getAlive());
                    }
                }
            }
        }
    }
}

void updateCells(vector<Cell> &cells) {
    Vector3Int offsets[26];
    size_t totalOffsets;
    Cell::clearCellCounts();
    if (NEIGHBORHOODS == MOORE) {
        // I don't know how to set array to a different array
        offsets[0] = { -1, -1, -1 };
        offsets[1] = { -1, -1, 0 };
        offsets[2] = { -1, -1, 1 };
        offsets[3] = { -1, 0, -1 };
        offsets[4] = { -1, 0, 0 };
        offsets[5] = { -1, 0, 1 };
        offsets[6] = { -1, 1, -1 };
        offsets[7] = { -1, 1, 0 };
        offsets[8] = { -1, 1, 1 };
        offsets[9] = { 0, -1, -1 };
        offsets[10] = { 0, -1, 0 };
        offsets[11] = { 0, -1, 1 };
        offsets[12] = { 0, 0, -1 };
        offsets[13] = { 0, 0, 1 };
        offsets[14] = { 0, 1, -1 };
        offsets[15] = { 0, 1, 0 };
        offsets[16] = { 0, 1, 1 };
        offsets[17] = { 1, -1, -1 };
        offsets[18] = { 1, -1, 0 };
        offsets[19] = { 1, -1, 1 };
        offsets[20] = { 1, 0, -1 };
        offsets[21] = { 1, 0, 0 };
        offsets[22] = { 1, 0, 1 };
        offsets[23] = { 1, 1, -1 };
        offsets[24] = { 1, 1, 0 };
        offsets[25] = { 1, 1, 1 };
        totalOffsets = 26;
    }
    else {
        offsets[0] = { 1, 0, 0 };
        offsets[1] = { -1, 0, 0 };
        offsets[2] = { 0, 1, 0 };
        offsets[3] = { 0, -1, 0 };
        offsets[4] = { 0, 0, 1 };
        offsets[5] = { 0, 0, -1 };
        totalOffsets = 6;
    }

    thread neighborThreads[threads];
    for (size_t i = 0; i < threads; i++) {
        int start = i * cellBounds / threads;
        int end = (i + 1) * cellBounds / threads;
        neighborThreads[i] = thread(updateNeighbors, std::ref(cells), start, end, offsets, totalOffsets);
    }
    for (size_t i = 0; i < threads; i++) {
        neighborThreads[i].join();
    }

    thread syncThreads[threads];
    for (size_t i = 0; i < threads; i++) {
        size_t start = i * totalCells / threads;
        size_t end = (i + 1) * totalCells / threads;
        syncThreads[i] = thread(syncCells, std::ref(cells), start, end);
    }
    for (size_t i = 0; i < threads; i++) {
        syncThreads[i].join();
    }
}


void drawCells(const vector<Cell> &cells, int divisor, DrawMode drawMode) {
    // A bit exessive to put this on the outside, but is saves doing cellBounds^3
    // extra checks at the cost of extra code
    switch (drawMode) {
        case DUAL_COLOR:
            for (int x = 0; x < cellBounds/divisor; x++) {
                for (int y = 0; y < cellBounds; y++) {
                    for (int z = 0; z < cellBounds; z++) {
                        cells[threeToOne(x, y, z)].drawDualColor();
                    }
                }
            }
            break;
        case RGB_CUBE:
            for (int x = 0; x < cellBounds/divisor; x++) {
                for (int y = 0; y < cellBounds; y++) {
                    for (int z = 0; z < cellBounds; z++) {
                        cells[threeToOne(x, y, z)].drawRGBCube();
                    }
                }
            }
            break;
        case DUAL_COLOR_DYING:
            for (int x = 0; x < cellBounds/divisor; x++) {
                for (int y = 0; y < cellBounds; y++) {
                    for (int z = 0; z < cellBounds; z++) {
                        cells[threeToOne(x, y, z)].drawDualColorDying();
                    }
                }
            }
            break;
        case SINGLE_COLOR:
            for (int x = 0; x < cellBounds/divisor; x++) {
                for (int y = 0; y < cellBounds; y++) {
                    for (int z = 0; z < cellBounds; z++) {
                        cells[threeToOne(x, y, z)].drawSingleColor();
                    }
                }
            }
            break;
        case CENTER_DIST:
            for (int x = 0; x < cellBounds/divisor; x++) {
                for (int y = 0; y < cellBounds; y++) {
                    for (int z = 0; z < cellBounds; z++) {
                        cells[threeToOne(x, y, z)].drawDist();
                    }
                }
            }
            break;
    }
}

void drawLeftBar(
    bool drawBounds,
    bool showHalf,
    bool paused,
    DrawMode drawMode,
    TickMode tickMode,
    int updateSpeed,
    int ticks,
    float growthRate,
    float deathRate,
    float cameraLat,
    float cameraLon
) {
    // I know there are a lot of parameters, but this vastly cleans the draw() function
    char dirs[2] = {
        (cameraLat > 0 ? 'N' : 'S'),
        (cameraLon > 0 ? 'W' : 'E')
    };

    string survivalText = "- Survival:";
    for (size_t i = 0; i < 27; i++) {
        if (SURVIVAL[i]) survivalText += " " + std::to_string(i);
    }

    string spawnText = "- Spawn:";
    for (size_t i = 0; i < 27; i++) {
        if (SPAWN[i]) spawnText += " " + std::to_string(i);
    }

    const DrawableText dts[] = {
        DrawableText("Controls:"),
        DrawableText("- Q/E : zoom in/out"),
        DrawableText("- W/S : rotate camera up/down"),
        DrawableText("- A/D : rotate camera left/right"),
        DrawableText("- R : re-randomize cells"),
        DrawableText("- B : show/hide bounds " + (string)(drawBounds ? "(on)" : "(off)")),
        DrawableText("- P : show/hide this bar (on)"),
        DrawableText("- C : toggle cross section view " + (string)(showHalf ? "(on)" : "(off)")),
        DrawableText("- Mouse click : pause/unpause " + (string)(paused ? "(paused)" : "(running)")),
        DrawableText("- Space : reset camera"),
        DrawableText("- Enter : toggle fullscreen"),
        DrawableText("- J : reload from JSON"),
        DrawableText("- O : toggle true fullscreen (not reccomended)"),
        DrawableText("- M : change between draw modes [" + textFromEnum(drawMode) + "]"),
        DrawableText("- U : change between tick modes [" + textFromEnum(tickMode) + "]"),
        (tickMode == MANUAL ? DrawableText("- X/Z : increase/decrease tick speed") : DrawableText("")),

        DrawableText("Simulation Info:"),
        DrawableText("- FPS: " + std::to_string(GetFPS())),
        DrawableText("- Ticks per sec: " + std::to_string(tickMode == FAST ? GetFPS() : updateSpeed)),
        DrawableText("- Total ticks ('time'): " + std::to_string(ticks)),
        DrawableText("- Total alive cells: " + std::to_string(Cell::getAliveCells())),
        DrawableText("- Growth Rate: " + std::to_string((int)((growthRate - 1.0f) * 100)) + "%"),
        DrawableText("- Death Rate: " + std::to_string((int)((deathRate - 1.0f) * 100)) + "%"),
        DrawableText("- Bound size: " + std::to_string(cellBounds)),
        DrawableText("- Threads: " + std::to_string(threads) + " (+ 2)"),
        DrawableText("- Camera pos: " + std::to_string((int)abs(cameraLat)) + dirs[0] + ", " + std::to_string(abs((int)cameraLon)) + dirs[1]),

        DrawableText("Rules:"),
        DrawableText(survivalText),
        DrawableText(spawnText),
        DrawableText("- State: " + std::to_string(STATE)),
        DrawableText("- Neighborhood: " + textFromEnum(NEIGHBORHOODS)),
    };

    const size_t lenTexts = sizeof(dts) / sizeof(dts[0]);

    DrawRectangle(10, 10, 290, lenTexts * 14 + 7, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 290, lenTexts * 14 + 7, BLUE);

    for (size_t i = 0; i < lenTexts; i++) {
        dts[i].draw(i);
    }
}

void draw(
    Camera3D camera,
    const vector<Cell> &cells,
    bool drawBounds,
    bool drawBar,
    bool showHalf,
    bool paused,
    DrawMode drawMode,
    TickMode tickMode,
    int updateSpeed, 
    int ticks,
    float growthRate,
    float deathRate,
    float cameraLat,
    float cameraLon)
 {
    // I know there are a lot of parameters, but this vastly cleans the main() function
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
            drawCells(cells, (int)showHalf + 1, drawMode);

            if (drawBounds) {
                if (showHalf) DrawCubeWires((Vector3){ -cellBounds/4.0f, 0, 0 }, cellBounds/2.0f, cellBounds, cellBounds, BLUE);
                else DrawCubeWires((Vector3){ 0, 0, 0 }, cellBounds, cellBounds, cellBounds, BLUE);
            }
        EndMode3D();
        if (drawBar) {
            drawLeftBar(drawBounds, showHalf, paused, drawMode, tickMode, updateSpeed, ticks, growthRate, deathRate, cameraLat, cameraLon);
        }
    EndDrawing();
}


void randomizeCells(vector<Cell> &cells) {
    for (size_t i = 0; i < totalCells; i++) {
        cells[i].reset();
    }
    // Only middle section has a spawn chance
    for (int x = cellBounds/3.0f; x < cellBounds * 2.0f/3.0f; x++) {
        for (int y = cellBounds/3.0f; y < cellBounds * 2.0f/3.0f; y++) {
            for (int z = cellBounds/3.0f; z < cellBounds * 2.0f/3.0f; z++) {
                cells[threeToOne(x, y, z)].randomizeState();
            }
        }
    }
    Cell::clearCellCounts();
}


vector<Cell> createCells() {
    vector<Cell> cells;
    cells.reserve(totalCells);
    for (int x = 0; x < cellBounds; x++) {
        for (int y = 0; y < cellBounds; y++) {
            for (int z = 0; z < cellBounds; z++) {
                cells.push_back(Cell({ x, y, z }));
            }
        }
    }
    return cells;
}


int main(void) {

    srand(time(NULL));

    const int screenWidth = 1200;
    const int screenHeight = 675;

    InitWindow(screenWidth, screenHeight, "3D Cellular Automata with Raylib");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    loadFromJSON();

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 1.75f * cellBounds;
    const float cameraMoveSpeed = 180.0f/4.0f;
    const float cameraZoomSpeed = cellBounds/10.0f;

    int ticks = 0;
    int lastAliveCells = Cell::getAliveCells();
    float growthRate = 1.0f;
    int lastDeadCells = Cell::getDeadCells();
    float deathRate = 1.0f;

    bool paused = false;
    bool drawBounds = false;
    bool showHalf = false;
    bool drawBar = true;
    DrawMode drawMode = DUAL_COLOR;
    TickMode tickMode = FAST;

    ToggleKey mouseTK;
    ToggleKey enterTK;
    ToggleKey xTK;
    ToggleKey zTK;
    ToggleKey bTK;
    ToggleKey cTK;
    ToggleKey mTK;
    ToggleKey uTK;
    ToggleKey pTK;
    ToggleKey oTK;
    ToggleKey jTK;

    int updateSpeed = 5;
    float frame = 0;

    vector<Cell> cells = createCells();
    randomizeCells(cells);
    vector<Cell> cells2 = vector<Cell>(cells);

    // Main game loop
    while (!WindowShouldClose()) {

        const float delta = GetFrameTime();
        frame += delta;

        if (IsKeyDown('W') || IsKeyDown(KEY_UP)) cameraLat += cameraMoveSpeed * delta;
        if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) cameraLat -= cameraMoveSpeed * delta;
        if (IsKeyDown('A') || IsKeyDown(KEY_LEFT)) cameraLon -= cameraMoveSpeed * delta;
        if (IsKeyDown('D')|| IsKeyDown(KEY_RIGHT)) cameraLon += cameraMoveSpeed * delta;
        if (IsKeyDown('Q') || IsKeyDown(KEY_PAGE_UP)) cameraRadius -= cameraZoomSpeed * delta;
        if (IsKeyDown('E') || IsKeyDown(KEY_PAGE_DOWN)) cameraRadius += cameraZoomSpeed * delta;
        if (IsKeyDown('R')) {
            randomizeCells(cells);
            ticks = 0;
        }
        if (mouseTK.down(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) paused = !paused;
        if (bTK.down(IsKeyPressed('B'))) drawBounds = !drawBounds;
        if (xTK.down(IsKeyDown('X') && tickMode == MANUAL)) updateSpeed++;
        if (zTK.down(IsKeyDown('Z') && tickMode == MANUAL && updateSpeed > 1)) updateSpeed--;
        if (cTK.down(IsKeyDown('C'))) showHalf = !showHalf;
        if (mTK.down(IsKeyDown('M'))) drawMode = (DrawMode)((drawMode + 1) % 5);
        if (uTK.down(IsKeyDown('U'))) tickMode = (TickMode)((tickMode + 1) % 3);
        if (pTK.down(IsKeyDown('P'))) drawBar = !drawBar;
        if (jTK.down(IsKeyDown('J'))) {
            int oldBounds = cellBounds;
            int oldState = STATE;
            loadFromJSON();
            cells2 = createCells();
            int start = (cellBounds - oldBounds) / 2;
            Vector3Int offset = { start, start, start };
            for (int x = 0; x < oldBounds; x++) {
                for (int y = 0; y < oldBounds; y++) {
                    for (int z = 0; z < oldBounds; z++) {
                        if (validCellIndex(x, y, z, offset)) {
                            size_t oldOneIdx = x * oldBounds * oldBounds + y * oldBounds  + z;
                            cells2[threeToOne(x + offset.x, y + offset.x, z + offset.z)].setHp(cells[oldOneIdx].getHp());
                        }
                    }
                }
            }
            cells = vector<Cell>(cells2);
            for (size_t i = 0; i < totalCells; i++) {
                cells[i].jsonStateUpdate(oldState);
            }
            cameraRadius = 1.75f * cellBounds;
        }
        if (IsKeyDown(KEY_SPACE)) {
            cameraLat = 20.0f;
            cameraLon = 20.0f;
            cameraRadius = 1.75f * cellBounds;
        }
        if (enterTK.down(IsKeyPressed(KEY_ENTER))) {
            if (GetScreenWidth() == screenWidth) MaximizeWindow();
            else RestoreWindow();
        }
        if (oTK.down(IsKeyPressed('O'))) {
 			ToggleFullscreen();
            if (IsWindowFullscreen()) {
                int display = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            }
            else  {
                SetWindowSize(screenWidth, screenHeight);
            }
        }

        if (cameraLat > 90) cameraLat = 89.99f;
        else if (cameraLat < -90) cameraLat = -89.99f;
        if (cameraLon > 180) cameraLon -= 360;
        else if (cameraLon < -180) cameraLon += 360;

        if (cameraRadius < 1) cameraRadius = 1.0f;
        camera.position = (Vector3){
            cameraRadius * cos(degreesToRadians(cameraLat)) * cos(degreesToRadians(cameraLon)),
            cameraRadius * cos(degreesToRadians(cameraLat)) * sin(degreesToRadians(cameraLon)),
            cameraRadius * sin(degreesToRadians(cameraLat))
        };

        if (!paused && (tickMode == FAST || frame >= 1.0f/updateSpeed)) {
            if (tickMode == DYNAMIC) {
                if (GetFPS() > targetFPS && updateSpeed < GetFPS()) updateSpeed++;
                else if (GetFPS() < targetFPS && updateSpeed > 1) updateSpeed--;
            }
            while (tickMode != FAST && frame >= 1.0/updateSpeed) frame -= 1.0/updateSpeed;

            cells2 = vector<Cell>(cells); // create copy to be updated in background
            thread updateThread(updateCells, std::ref(cells2));

            draw(camera, cells, drawBounds, drawBar, showHalf, paused, drawMode, tickMode, updateSpeed, ticks, growthRate, deathRate, cameraLat, cameraLon);

            updateThread.join();
            cells = vector<Cell>(cells2); // copy the updated cells to the main cells
            
            ticks++;
            growthRate = Cell::getAliveCells() / (float)lastAliveCells;
            lastAliveCells = Cell::getAliveCells();
            deathRate = Cell::getDeadCells() / (float)lastDeadCells;
            lastDeadCells = Cell::getDeadCells();
        }
        else {
            draw(camera, cells, drawBounds, drawBar, showHalf, paused, drawMode, tickMode, updateSpeed, ticks, growthRate, deathRate, cameraLat, cameraLon);
        }

    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
