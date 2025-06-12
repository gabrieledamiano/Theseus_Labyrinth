#pragma once
#ifndef CELL_H
#define CELL_H

// La nostra struct, definita in un unico posto sicuro.
struct Cell {
    bool visited = false;
    bool wall = true;
};

#endif // CELL_H