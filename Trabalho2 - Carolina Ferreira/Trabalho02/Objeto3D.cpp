// Objeto3D.cpp
#include <cstdlib>
#include "Objeto3D.h"
#include "Ponto.h"

void TPoint::Set(float x, float y, float z) {
    X = x;
    Y = y;
    Z = z;
}

void TPoint::Imprime() {
    std::cout << "X: " << X << " Y: " << Y << " Z: " << Z;
}

void TTriangle::Imprime() {
    std::cout << "P1 ";
    P1.Imprime();
    std::cout << std::endl;
    std::cout << "P2 ";
    P2.Imprime();
    std::cout << std::endl;
    std::cout << "P3 ";
    P3.Imprime();
    std::cout << std::endl;
}

Objeto3D::Objeto3D() {
    faces = NULL;
    nFaces = 0;
}

unsigned int Objeto3D::getNFaces() {
    return nFaces;
}

void Objeto3D::LeObjeto(char *Nome) {
    std::ifstream arq(Nome, std::ios::in);
    if (!arq) {
        std::cout << "Erro na abertura do arquivo " << Nome << "." << std::endl;
        exit(1);
    }
    arq >> nFaces;
    faces = new TTriangle[nFaces];
    float x, y, z;
    for (int i = 0; i < nFaces; i++) {
        arq >> x >> y >> z;
        faces[i].P1.Set(x, y, z);

        arq >> x >> y >> z;
        faces[i].P2.Set(x, y, z);

        arq >> x >> y >> z;
        faces[i].P3.Set(x, y, z);
    }
}

void Objeto3D::ExibeObjeto() {
    for (int i = 0; i < nFaces; i++) {
        Ponto A = Ponto(faces[i].P2.X, faces[i].P2.Y, faces[i].P2.Z);
        Ponto B = Ponto(faces[i].P3.X, faces[i].P3.Y, faces[i].P3.Z);
        Ponto vetNormal;
        ProdVetorial(A, B, vetNormal);
        glBegin(GL_TRIANGLES);
            glNormal3f(vetNormal.x, vetNormal.y, vetNormal.z);
            glVertex3f(faces[i].P1.X, faces[i].P1.Y, faces[i].P1.Z);
            glVertex3f(faces[i].P2.X, faces[i].P2.Y, faces[i].P2.Z);
            glVertex3f(faces[i].P3.X, faces[i].P3.Y, faces[i].P3.Z);
        glEnd();
    }
}
