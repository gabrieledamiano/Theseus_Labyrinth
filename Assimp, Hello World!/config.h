#pragma once
#ifndef CONFIG_H
#define CONFIG_H

// Dimensioni della finestra
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Dimensioni del labirinto
const int MAP_SIZE_ROWS = 40;
const int MAP_SIZE_COLS = 20;
const int MAZE_WIDTH = MAP_SIZE_COLS;
const int MAZE_HEIGHT = MAP_SIZE_ROWS;

// Proprietà fisiche del labirinto
const float CELL_SIZE = 2.0f;
const float WALL_HEIGHT = 3.0f;

// --- COSTANTI PER L'USCITA ---
const float EXIT_Z_THRESHOLD = 39.0f * CELL_SIZE; // La linea da attraversare sull'asse Z
const float EXIT_X_MIN = 5.0f * CELL_SIZE;        // Il bordo sinistro del corridoio di uscita
const float EXIT_X_MAX = 7.0f * CELL_SIZE;        // Il bordo destro del corridoio di uscita

// Proprietà della camera
const float CAMERA_HEIGHT = 1.5f;
const float CAMERA_COLLISION_RADIUS = 0.2f;

#endif // CONFIG_H