// #include "raylib.h"
// #include <math.h>

// #define PI 3.14159265358979323846


// float degreesToRadians(float degrees) {
//     return degrees * PI / 180.0f;
// }


// int main(void)
// {
//     // Initialization
//     //--------------------------------------------------------------------------------------
//     const int screenWidth = 800;
//     const int screenHeight = 450;

//     InitWindow(screenWidth, screenHeight, "3D Cellular Automata");

//     // Define the camera to look into our 3d world
//     Camera3D camera = { 0 };
//     camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
//     camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
//     camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
//     camera.fovy = 45.0f;                                // Camera field-of-view Y
//     camera.projection = CAMERA_PERSPECTIVE;                   // Camera mode type

//     float cameraLat = 20.0f;
//     float cameraLon = 20.0f;
//     float cameraRadius = 20.0f;
//     float cameraMoveSpeed = 1.0f;
//     float cameraZoomSpeed = 0.25f;

//     Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

//     SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

//     // Main game loop
//     while (!WindowShouldClose())        // Detect window close button or ESC key
//     {
//         if (IsKeyDown('W')) cameraLat += cameraMoveSpeed;
//         else if (IsKeyDown('S')) cameraLat -= cameraMoveSpeed;
//         if (IsKeyDown('A')) cameraLon -= cameraMoveSpeed;
//         else if (IsKeyDown('D')) cameraLon += cameraMoveSpeed;
//         if (IsKeyDown('Q')) cameraRadius -= cameraZoomSpeed;
//         else if (IsKeyDown('E')) cameraRadius += cameraZoomSpeed;

//         camera.position = (Vector3){
//             cameraRadius * cos(degreesToRadians(cameraLat)) * cos(degreesToRadians(cameraLon)),
//             cameraRadius * cos(degreesToRadians(cameraLat)) * sin(degreesToRadians(cameraLon)),
//             cameraRadius * sin(degreesToRadians(cameraLat))
//         };

//         // Draw
//         BeginDrawing();

//             ClearBackground(RAYWHITE);

//             BeginMode3D(camera);

//                 DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
//                 DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, GREEN);

//             EndMode3D();

//             DrawRectangle( 10, 10, 320, 133, Fade(SKYBLUE, 0.5f));
//             DrawRectangleLines( 10, 10, 320, 133, BLUE);

//             DrawText("Camera controls:", 20, 20, 10, BLACK);
//             DrawText("- Q to zoom in, E to zoom out", 40, 40, 10, DARKGRAY);
//             DrawText("- W to rotate camera up, S to rotate down", 40, 60, 10, DARKGRAY);
//             DrawText("- A to rotate left, D to rotate right", 40, 80, 10, DARKGRAY);
//             // DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 40, 100, 10, DARKGRAY);
//             // DrawText("- Z to zoom to (0, 0, 0)", 40, 120, 10, DARKGRAY);

//         EndDrawing();
//     }


//     CloseWindow();        // Close window and OpenGL context
//     return 0;
// }