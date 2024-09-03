#ifndef OBJETO3D_H
#define OBJETO3D_H

#include <iostream>
#include <fstream>
#include <glut.h>

typedef struct {
    float X, Y, Z;
    void Set(float x, float y, float z);
    void Imprime();
} TPoint;

typedef struct {
    TPoint P1, P2, P3;
    void Imprime();
} TTriangle;

// Classe para armazenar um objeto 3D
class Objeto3D {
    TTriangle *faces; // vetor de faces
    unsigned int nFaces; // Variavel que armazena o numero de faces do objeto
public:
    Objeto3D();
    unsigned int getNFaces();
    void LeObjeto (char *Nome);
    void ExibeObjeto();
};

#endif