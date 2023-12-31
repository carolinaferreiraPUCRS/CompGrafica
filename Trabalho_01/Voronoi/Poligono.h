//
//  Poligono.hpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 18/08/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef Poligono_hpp
#define Poligono_hpp

#include <iostream>
using namespace std;


#ifdef WIN32
#include <windows.h>
#include <glut.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Envelope.h"
#include <vector>

class Poligono
{
    vector <Ponto> Vertices;
    vector <Poligono *> Vizinhos;
    vector <Envelope> Envelopes;
    Ponto Min, Max;
public:
    Poligono();
    Ponto getVertice(int);
    Poligono* getVizinho(int);
    Envelope getEnvelope(int i);
    unsigned long getNVertices();
    unsigned long getNVizinhos();
    void insereVertice(Ponto);
    void insereVertice(Ponto p, int pos);
    void insereVizinho(Poligono *P);
    void insereVizinho(Poligono *P, int pos);
    void insereEnvelope(Envelope e);
    void desenhaPoligono();
    void desenhaVertices();
    void pintaPoligono();
    void imprime();
    void atualizaLimites();
    void obtemLimites(Ponto &Min, Ponto &Max);
    void LePoligono(const char *nome);
    void desenhaAresta(int n);
    void getAresta(int i, Ponto &P1, Ponto &P2);
    bool pontoEstaDentro(Ponto p);

};

#endif
