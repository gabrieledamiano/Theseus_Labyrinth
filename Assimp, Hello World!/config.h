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
const float CELL_SIZE = 1.5f;
const float WALL_HEIGHT = 3.0f;

// Proprietà della camera
const float CAMERA_HEIGHT = 1.5f;
const float CAMERA_COLLISION_RADIUS = 0.2f;

#endif // CONFIG_H