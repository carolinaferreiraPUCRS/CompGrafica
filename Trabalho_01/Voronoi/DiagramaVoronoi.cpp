//
//  DiagramaVoronoi.cpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 23/08/23.
//  Copyright © 2023 Márcio Sarroglia Pinho. All rights reserved.
//

#include "DiagramaVoronoi.h"

ifstream input;            // ofstream arq;

Voronoi::Voronoi()
{

}
Poligono Voronoi::LeUmPoligono()
{
    Poligono P;
    unsigned int qtdVertices;
    input >> qtdVertices;  // arq << qtdVertices
    for (int i=0; i< qtdVertices; i++)
    {
        double x,y;
        // Le um ponto
        input >> x >> y;
        Ponto(x, y).imprime();
        if(!input)
        {
            cout << "Fim inesperado da linha." << endl;
            break;
        }
        P.insereVertice(Ponto(x,y));
    }
    cout << "Poligono lido com sucesso!" << endl;
    return P;
}

void Voronoi::LePoligonos(const char *nome)
{
    input.open(nome, ios::in); //arq.open(nome, ios::out);
    if (!input)
    {
        cout << "Erro ao abrir " << nome << ". " << endl;
        exit(0);
    }
    string S;

    input >> qtdDePoligonos;
    cout << "qtdDePoligonos:" << qtdDePoligonos << endl;
    Ponto A, B;
    Diagrama[0] = LeUmPoligono();
    Diagrama[0].obtemLimites(Min, Max);// obtem o envelope do poligono
    for (int i=1; i< qtdDePoligonos; i++)
    {
        Diagrama[i] = LeUmPoligono();
        Diagrama[i].obtemLimites(A, B); // obtem o envelope do poligono

        Min = ObtemMinimo (A, Min);
        Max = ObtemMaximo (B, Max);
    }
    cout << "Lista de Poligonos lida com sucesso!" << endl;

}

Poligono Voronoi::getPoligono(int i)
{
    if (i >= qtdDePoligonos)
    {
        cout << "Nro de Poligono Inexistente" << endl;
        return Diagrama[0];
    }
    return Diagrama[i];
}
unsigned int Voronoi::getNPoligonos()
{
    return qtdDePoligonos;
}
void Voronoi::obtemLimites(Ponto &min, Ponto &max)
{
    min = this->Min;
    max = this->Max;
}

void Voronoi::obtemVizinhosDasArestas()
{
    for (int i = 0; i < this->getNPoligonos(); i++) {
        Poligono p1 = this->getPoligono(i);
        Poligono p2 = this->getPoligono((i + 1) % this->getNPoligonos());

        cout << "Poligono 1: " << i << endl;
        cout << "Poligono 2: " << ((i + 1) % this->getNPoligonos()) << endl;

        for (int j = 0; j < p1.getNVertices(); j++) {
            Ponto p1a = p1.getVertice(j);
            Ponto p1b = p1.getVertice((j + 1) % p1.getNVertices());

            cout << "Aresta " << j << " de " << i << endl;

            for (int k = 0; k < p2.getNVertices(); k++) {
                Ponto p2a = p2.getVertice(k);
                Ponto p2b = p2.getVertice((k + 1) % p2.getNVertices());
                if (p1a == p2b && p1b == p2a) {
                    p1.insereVizinho(&p2);
                    p2.insereVizinho(&p1);

                    cout << "O poligono " << i << " e vizinho do poligono " << ((i + 1) % p1.getNVertices()) << endl;
                    cout << "O poligono " << ((i + 1) % p1.getNVertices()) << " e vizinho do poligono " << i << endl;

                    cout << "Aresta " << j << " de " << i << " e vizinha da aresta " << k << " de " << ((i + 1) % p1.getNVertices()) << endl;
                    cout << "Aresta " << k << " de " << ((i + 1) % p1.getNVertices()) << " e vizinha da aresta " << j << " de " << i << endl;
                }
            }
        }
    }
}
