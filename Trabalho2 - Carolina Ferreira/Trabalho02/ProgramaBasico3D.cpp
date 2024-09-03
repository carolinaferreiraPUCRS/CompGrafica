// **********************************************************************
// PUCRS/Escola Politecnica
// COMPUTACAO GRAFICA
//
// Programa basico para criar aplicacoes 3D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
//
// Carolina Michel Ferreira
// carolina.michel@edu.pucrs.br
// **********************************************************************

#include <iostream>
#include <cmath>
#include <ctime>

using namespace std;


#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"
#include "Ponto.h"
#include "ImageClass.h"
#include "SOIL/SOIL.h"
#include "Objeto3D.h"
Temporizador T;
double AccumDeltaT=0;


GLfloat AspectRatio;
GLuint floorTexture, wallTexture, vehicleTexture, cannonTexture;
typedef struct {
    Ponto position;
    bool isFriend;
    bool isAlive;
} Frenemy;

// Controle do modo de projecao
// 0: Projecao Paralela Ortografica; 1: Projecao Perspectiva
// A funcao "PosicUser" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'p'
int ModoDeProjecao = 1;


// Controle do modo de projecao
// 0: Wireframe; 1: Faces preenchidas
// A funcao "Init" utiliza esta variavel. O valor dela eh alterado
// pela tecla 'e'
int ModoDeExibicao = 1;

double nFrames = 0;
double TempoTotal = 0;

const int wallHeight = 15;
const int sceneWidth = 25;
const int sceneDepth = 50;
const int frenemiesCount = 20;
int enemiesCount;
float score;
Ponto WallPosition;
bool wallGrid[sceneWidth][wallHeight];
Objeto3D frenemy;
Frenemy* frenemies;
Ponto CameraRelativePosition;
Ponto VehicleSize;
float vehicleMovementDistance;
Ponto CannonSize;
Ponto CannonRelativePosition;
Ponto VehiclePosition;
Ponto VehicleAngle;
Ponto VehicleDirection;
Ponto CannonAngle;
Ponto CannonDirection;
float cannonForce;
Ponto BezierPoints3[3];
bool isShooting;
float projectileDisplacement;
bool sideView;
bool gameOver;

void Options()
{
    cout << "========================================" << endl;
    cout << "Bem-vindo ao jogo Guerra dos Cachorros!" << endl;
    cout << "Voce este em uma missao para salvar os seus cachorros que foram abduzidos por uma trupe de cachorros malignos. Faca o seu melhor e garanta a seguranca dos seus amigos nesta missao." << endl;
    cout << "************* CONTROLES DO JOGO: ***************" << endl;
    cout << "Pressione W/S para andar para frente ou para tras" << endl;
    cout << "Pressione a para rotacionar o veiculo para a esquerda" << endl;
    cout << "Pressione A para rotacionar o veiculo para a direita" << endl;
    cout << "Pressione b/B para rotacionar o canhao para cima ou para baixo" << endl;
    cout << "Pressione F/f para aumentar/diminuir a forca do canhao" << endl;
    cout << "Pressione ESPACO para atirar" << endl;
    cout << "Pressione V para ver sua pontuacao e o numero de inimigos restantes" << endl;
    cout << "Pressione H para ver estas instrucoes novamente" << endl;
    cout << "Pressione T para ver o cenário lateralmente" << endl;
    cout << "Pressione R para reiniciar" << endl;
    cout << "Pressione ESC para sair" << endl;
    cout << "========================================" << endl;
}

void MygluPerspective(float fieldOfView, float aspect, float zNear, float zFar) {
    // https://stackoverflow.com/questions/2417697/gluperspective-was-removed-in-opengl-3-1-any-replacements/2417756#2417756
    // The following code is a fancy bit of math that is equivilant to calling:
    // gluPerspective(fieldOfView/2.0f, width/height, 0.1f, 255.0f)
    // We do it this way simply to avoid requiring glu.h
    // GLfloat zNear = 0.1f;
    // GLfloat zFar = 255.0f;
    // GLfloat aspect = float(width)/float(height);
    GLfloat fH = tan(float(fieldOfView / 360.0f * 3.14159f)) * zNear;
    GLfloat fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void PosicUser()
{
    // Define os parametros da projecao Perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Projecao perspectiva
    MygluPerspective(90, AspectRatio, 0.01, 50);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float xViewer = -13;
    float yViewer = 20;
    float zViewer = 25;
    float xTarget = 25;
    float yTarget = 0;
    float zTarget = 25;
    if (!sideView) {
        xViewer = VehiclePosition.x + CameraRelativePosition.x;
        yViewer = VehiclePosition.y + CameraRelativePosition.y;
        zViewer = VehiclePosition.z + CameraRelativePosition.z;
        xTarget = VehiclePosition.x;
        yTarget = VehiclePosition.y;
        zTarget = VehiclePosition.z;
    }
    gluLookAt(
        // Posicao da camera
        xViewer, yViewer, zViewer,
        // Posicao do alvo
        xTarget, yTarget, zTarget,
        // Vetor ViewUp
        0.0f, 1.0f, 0.0f
    );
}

bool HitVehicle()
{
    return CannonAngle.x <= -90;
}

bool HitWall(Ponto P1)
{
    int x = round(P1.x);
    int y = round(P1.y);
    int z = round(P1.z);
    if (x < 0 || x >= sceneWidth || z != sceneDepth / 2 || y < 0 || y >= wallHeight)
        return false;
    return wallGrid[x][y];
}
bool VehicleCanMove(Ponto CurrentPoint, Ponto DesiredPoint)
{
    if (DesiredPoint.x < 0 || DesiredPoint.x > sceneWidth)
        return false;
    if (DesiredPoint.z < 0 || DesiredPoint.z > sceneDepth)
        return false;
    if (HitWall(DesiredPoint))
        return false;
    return true;
}

void DrawFrenemies()
{
    for (int i = 0; i < frenemiesCount; i++) {
        if (frenemies[i].isAlive) {
            Ponto position = frenemies[i].position;
            glPushMatrix();
                float x = position.x;
                float y = position.y;
                float z = position.z;
                glTranslatef(x, y, z);
                glRotatef(0, 1, 0, 0);
                glScalef(0.2, 0.2, 0.2);
                if (frenemies[i].isFriend)
                    defineCor(PaleGreen);
                else
                    defineCor(OrangeRed);
                frenemy.ExibeObjeto();
            glPopMatrix();
        }
    }
}


void arrow_keys(int a_keys, int x, int y)
{
	switch (a_keys) {
        // When Up Arrow Is Pressed...
		case GLUT_KEY_UP:
			glutFullScreen();
			break;
        // When Down Arrow Is Pressed...
	    case GLUT_KEY_DOWN:
			glutInitWindowSize(700, 500);
			break;
		default:
			break;
	}
}

void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    // fixa a atualizacao da tela em 30
    if (AccumDeltaT > 1.0/30)
    {
        AccumDeltaT = 0;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        TempoTotal = 0;
        nFrames = 0;
    }
}

void DefineLuz(void)
{
    // Define cores para um objeto dourado
    GLfloat LuzAmbiente[] = {0.4, 0.4, 0.4};
    GLfloat LuzDifusa[] = {0.7, 0.7, 0.7};
    GLfloat LuzEspecular[] = {0.9f, 0.9f, 0.9};
    GLfloat PosicaoLuz0[] = {0.0f, 3.0f, 5.0f};
    GLfloat Especularidade[] = {1.0f, 1.0f, 1.0f};

    // ****************  Fonte de Luz 0
    glEnable(GL_COLOR_MATERIAL);

    // Habilita o uso de iluminacao
    glEnable(GL_LIGHTING);

    // Ativa o uso da luz ambiente
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LuzAmbiente);
    // Define os parametros da luz numero Zero
    glLightfv(GL_LIGHT0, GL_AMBIENT, LuzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LuzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, LuzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, PosicaoLuz0);
    glEnable(GL_LIGHT0);

    // Ativa o "Color Tracking"
    glEnable(GL_COLOR_MATERIAL);

    // Define a reflectancia do material
    glMaterialfv(GL_FRONT,GL_SPECULAR, Especularidade);

    // Define a concentracao do brilho.
    // Quanto maior o valor do Segundo parametro, mais
    // concentrado sera o brilho. (Valores validos: de 0 a 128)
    glMateriali(GL_FRONT, GL_SHININESS, 51);
}

void reshape(int w, int h)
{
	// Evita divisao por zero, no caso de uam janela com largura 0.
	if (h == 0) h = 1;
    // Ajusta a relacao entre largura e altura para evitar distorcao na imagem.
    // Veja funcao "PosicUser".
	AspectRatio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Seta a viewport para ocupar toda a janela
    glViewport(0, 0, w, h);
    //cout << "Largura" << w << endl;

	PosicUser();
}

void BreakWall(Ponto P1) {
    int x = round(P1.x);
    int y = round(P1.y);
    int z = round(P1.z);
    wallGrid[x][y] = false;

    // Quebra vizinho superior esquerdo
    if (x > 0 && y < wallHeight - 1)
    {
        wallGrid[x - 1][y + 1] = false;
    }

    // Quebra vizinho esquerdo
    if (x > 0)
    {
        wallGrid[x - 1][y] = false;
    }

    // Quebra vizinho inferior esquerdo
    if (x > 0 && y > 0)
    {
        wallGrid[x - 1][y - 1] = false;
    }

    // Quebra vizinho superior
    if (y < wallHeight - 1)
    {
        wallGrid[x][y + 1] = false;
    }

    // Quebra vizinho inferior
    if (y > 0)
    {
        wallGrid[x][y - 1] = false;
    }

    // Quebra vizinho superior direito
    if (x < sceneWidth - 1 && y < wallHeight - 1)
    {
        wallGrid[x + 1][y + 1] = false;
    }

    // Quebra vizinho direito
    if (x < sceneWidth - 1)
    {
        wallGrid[x + 1][y] = false;
    }

    // Quebra vizinho inferior direito
    if (x < sceneWidth - 1 && y > 0)
    {
        wallGrid[x + 1][y - 1] = false;
    }    
}

bool HitFloor(Ponto P1)
{
    return P1.y < 0;
}

int HitFrenemy(Ponto P1, Ponto Offset)
{
    float offset = 1.0f;
    for (int i = 0; i < frenemiesCount; i++) {
        if (frenemies[i].isAlive) {
            Ponto position = frenemies[i].position;
             if (P1.x >= position.x - Offset.x && P1.x <= position.x + Offset.x &&
                P1.y >= position.y - Offset.y && P1.y <= position.y + Offset.y &&
                P1.z >= position.z - Offset.z && P1.z <= position.z + Offset.z) {
                    frenemies[i].isAlive = false;
                    return frenemies[i].isFriend ? 1 : 2;
            }
        }
    }
    return 0;
}
void DrawTile()
{
    // Desenha quadrado preenchido
     glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-0.5f, 0.0f, -0.5f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-0.5f, 0.0f, 0.5f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(0.5f, 0.0f, 0.5f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(0.5f, 0.0f, -0.5f);
    glEnd();

    defineCor(MediumGoldenrod);
    glBegin(GL_LINE_STRIP);
        glNormal3f(0,1,0);
        glVertex3f(-0.5f,  0.0f, -0.5f);
        glVertex3f(-0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f,  0.5f);
        glVertex3f( 0.5f,  0.0f, -0.5f);
    glEnd();
}

void DrawFloor()
{
    glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glTranslated(0, -1, 0);
        for (int x = 0; x < sceneWidth; x++)
        {
            glPushMatrix();
            for (int z = 0; z < sceneDepth; z++)
            {
                DrawTile();
                glTranslated(0, 0, 1);
            }
            glPopMatrix();
            glTranslated(1, 0, 0);
        }
    glPopMatrix();
}

void DrawWall()
{
    glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glTranslated(WallPosition.x, WallPosition.y, WallPosition.z);
        glRotatef(-90, 1, 0, 0);
        glPushMatrix();
            for (int x = 0; x < sceneWidth; x++)
            {
                glPushMatrix();
                    for (int z = 0; z < wallHeight; z++)
                    {
                        if (wallGrid[x][z])
                            DrawTile();
                        glTranslated(0, 0, 1);
                    }
                glPopMatrix();
                glTranslated(1, 0, 0);
            }
        glPopMatrix();
    glPopMatrix();
}

void DesenhaParalelepipedo(float largura, float altura, float profundidade)
{
    glPushMatrix();
        glTranslatef(0, 0, 0);
        glScalef(largura, altura, profundidade);
        // glutSolidCube(1);
        float tamAresta = 1.0f;
        glBegin(GL_QUADS);
            // Front Face
            glNormal3f(0, 0, 1);
            glTexCoord2f(0.5f, 0.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 0.5f);
            glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
            glTexCoord2f(0.5f, 0.5);
            glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);

            // Back Face
            glNormal3f(0, 0, -1);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);

            // Top Face
            glNormal3f(0, 1, 0);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);

            // Bottom Face
            glNormal3f(0, -1, 0);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);

            // Right face
            glNormal3f(1, 0, 0);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(tamAresta / 2, tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(tamAresta / 2, tamAresta / 2, tamAresta / 2);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(tamAresta / 2, -tamAresta / 2, tamAresta / 2);

            // Left Face
            glNormal3f(-1, 0, 0);
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, -tamAresta / 2);
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(-tamAresta / 2, -tamAresta / 2, tamAresta / 2);
            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(-tamAresta / 2, tamAresta / 2, tamAresta / 2);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-tamAresta / 2, tamAresta / 2, -tamAresta / 2);
        glEnd();
    glPopMatrix();
}

void DrawVehicle()
{
    glPushMatrix();
        // Base
        glTranslatef(VehiclePosition.x, VehiclePosition.y, VehiclePosition.z);
        glRotatef(VehicleAngle.y, 0, 1, 0);
        glBindTexture(GL_TEXTURE_2D, vehicleTexture);
        DesenhaParalelepipedo(VehicleSize.x, VehicleSize.y, VehicleSize.z);

        // Canhão
        glTranslatef(CannonRelativePosition.x, CannonRelativePosition.y, CannonRelativePosition.z);
        glRotatef(CannonAngle.x, 1, 0, 0);
        glBindTexture(GL_TEXTURE_2D, cannonTexture);
        DesenhaParalelepipedo(CannonSize.x, CannonSize.y, CannonSize.z);
    glPopMatrix();
}

void MoveVehicle(unsigned char key)
{
    Ponto Direction = Ponto(0, 0, 1);
    Direction.rotacionaY(VehicleAngle.y);
    Ponto DistanceTraveled = Direction * vehicleMovementDistance;
    if (key == 's' || key == 'S')
        DistanceTraveled = DistanceTraveled * -1;
    Ponto NewPosition = VehiclePosition + DistanceTraveled;
    if ((NewPosition.x < 0 || NewPosition.x > sceneWidth) || (NewPosition.z < 0 || NewPosition.z > sceneDepth))
    {
        cout << "Movimento invalido, veiculo nao pode sair da pista" << endl;
        return;
    }
    if (!VehicleCanMove(VehiclePosition, NewPosition))
    {
        cout << "Movimento invalido, veiculo nao pode colidir com paredes" << endl;
        return;
    }
    Ponto Offset = Ponto(2, 10, 2);
    int frenemy = HitFrenemy(NewPosition, Offset);
    if (frenemy != 0)
    {
        bool wasFrenemy = frenemy == 1;
        if (wasFrenemy)
        {
            cout << "Ah nao! Voce atropelou um cachorro aliado \t-10 pontos" << endl;
            score -= 10.0f;
        }
        else
        {
            cout << "Parabens! Voce atropelou um cachorro maligno \t+10 pontos" << endl;
            score += 10.0f;
            enemiesCount--;

            if (enemiesCount == 0)
            {
                cout << "Parabens! Voce matou todos os cachorros da trupe inimiga" << endl;
                gameOver = true;
            }
        }
    }
    VehiclePosition = NewPosition;
}

void RotateVehicle(unsigned char key) {
    if (key == 'a')
    {
        VehicleAngle.y -= 1.0f;
    }
    if (key == 'A')
    {
        VehicleAngle.y += 1.0f;
    }
}

void RotateCannon(unsigned char key) {
    if (key == 'b')
    {
        if (CannonAngle.x > -90)
        {
            CannonAngle.x -= 2.0f;
        }
    }
    if (key == 'B')
    {
        if (CannonAngle.x < 45)
        {
            CannonAngle.x += 2.0f;
        }
    }
}

void ShootProjectile()
{
    CannonDirection = Ponto(0, 0, 1);
    CannonDirection.rotacionaX(CannonAngle.x);
    CannonDirection.rotacionaY(VehicleAngle.y);
    // No material de apoio, "B"
    Ponto CannonPosition = VehiclePosition + CannonRelativePosition;
    Ponto ProjectileDirection = CannonRelativePosition + CannonDirection * cannonForce;
    float Distance = 2 * cannonForce * cos(CannonAngle.x * M_PI / 180);
    // No material de apoio, "C"
    Ponto Target = CannonPosition + Ponto(0, CannonPosition.y - 5, Distance);
    Target.rotacionaY(VehicleAngle.y);

    BezierPoints3[0] = CannonPosition;
    BezierPoints3[1] = CannonPosition + ProjectileDirection * 0.5;
    BezierPoints3[2] = Target;

    projectileDisplacement = 0.0f;
}

Ponto CalculateBezier3(Ponto PC[], double t)
{
    double UmMenosT = 1 - t;
    Ponto P = PC[0] * UmMenosT * UmMenosT + PC[1] * 2 * UmMenosT * t + PC[2] * t * t;
    return P;
}

void DrawProjectile()
{
    double DeltaT = 1.0 / 50;
    while (projectileDisplacement < 1.0)
    {
        Ponto P = CalculateBezier3(BezierPoints3, projectileDisplacement);

        if (HitVehicle())
        {
            cout << "Voce se matou " << endl;
            cout << "Final Score: " << score << endl;
            score = -999.9f;
            projectileDisplacement = 1.0f;
            break;
        }

       if (HitWall(P))
       {
            cout << "Parabens! Voce acertou no paredao +5 pontos" << endl;
            BreakWall(P);
            score += 5.0f;
            projectileDisplacement = 1.0f;
            break;
        }

        if (HitFloor(P))
        {
            cout << "Opa! Voce atirou no chao -5 pontos" << endl;
            score -= 5.0f;
            projectileDisplacement = 1.0f;
            break;
        }

        Ponto Offset = Ponto(2, 10, 2);
        int frenemy = HitFrenemy(P, Offset);
        if (frenemy != 0)
        {
            bool wasFriend = frenemy == 1;
            if (wasFriend)
            {
                cout << "Ah nao! Voce matou um cachorro aliado -10 pontos" << endl;
                score -= 10.0f;
            }
            else
            {
                cout << "Parabens! Voce matou um cachorro maligno +10 pontos" << endl;
                score += 10.0f;
                enemiesCount--;

                if (enemiesCount == 0)
                {
                    cout << "Parabens! Voce matou todos os cachorros da trupe inimiga" << endl;
                    gameOver = true;
                }
            }
            projectileDisplacement = 1.0f;
            break;
        }

        glPushMatrix();
            glColor3f(0.0f, 0.0f, 0.0f);
            glTranslated(P.x, P.y, P.z);
            glutSolidSphere(CannonSize.x, 30, 30);
        glPopMatrix();
        projectileDisplacement += DeltaT;
    }
}

bool handleGameOver()
{
    if (!gameOver)
        return false;

    cout << "Game Over" << endl;
    cout << "Pontuacao final: " << score << endl;
    cout << "Pressione ESC para sair" << endl;
    cout << "Pressione T para ver o cenário lateralmente" << endl;
    cout << "Pressione R para reiniciar" << endl;

    return true;
}

bool KillFrenemy(Frenemy* frenemy)
{
    frenemy -> isAlive = false;
    return frenemy -> isFriend;
}

GLuint LoadTexture(const char *textureName)
{
    GLenum errorCode;
    GLuint IdTEX;
    // Habilita o uso de textura
    glEnable(GL_TEXTURE_2D);

    // Define a forma de armazenamento dos pixels na textura (1= alihamento por byte)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Gera um identificar para a textura
    glGenTextures(1, &IdTEX); //  vetor que guardas os numeros das texturas
    errorCode = glGetError();
    if (errorCode == GL_INVALID_OPERATION) {
        cout << "Erro: glGenTextures chamada entre glBegin/glEnd." << endl;
        return -1;
    }

    // Define que tipo de textura sera usada
    // GL_TEXTURE_2D ==> define que sera usada uma textura 2D (bitmaps)
    // texture_id[OBJETO_ESQUERDA]  ==> define o numero da textura
    glBindTexture(GL_TEXTURE_2D, IdTEX);

    // Carrega a imagem
    ImageClass Img;

    int r = Img.Load(textureName);
    if (!r) {
        cout << "Erro lendo imagem " << textureName << endl;
        exit(1);
    }

    int level = 0;
    int border = 0;

    // Envia a textura para OpenGL, usando o formato apropriado
    int format;
    format = GL_RGB;
    if (Img.Channels() == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, level, format,
                 Img.SizeX(), Img.SizeY(),
                 border, format,
                 GL_UNSIGNED_BYTE, Img.GetImagePtr());

    errorCode = glGetError();
    if (errorCode == GL_INVALID_OPERATION) {
        cout << "Erro: glTexImage2D chamada entre glBegin/glEnd." << endl;
        return -1;
    }

    if (errorCode != GL_NO_ERROR) {
        cout << "Houve algum erro na criacao da textura." << endl;
        return -1;
    }

    // Ajusta os filtros iniciais para a textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Img.Delete();

    cout << "Carga de textura OK." << endl;
    return IdTEX;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DefineLuz();

	PosicUser();

	glMatrixMode(GL_MODELVIEW);

    DrawFloor();

    DrawWall();

    DrawVehicle();

    DrawProjectile();

    DrawFrenemies();

	glutSwapBuffers();
}

void init(void)
{
    // Fundo de tela azul claro
    glClearColor(0.6156862745f, 0.8980392157f, 0.9803921569f, 1.0f);

    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    // glShadeModel(GL_FLAT);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    if (ModoDeExibicao)
    {
        floorTexture = LoadTexture("gramaMinecraft.jpg");
        wallTexture = LoadTexture("paredemadeira.jpg");
        vehicleTexture = LoadTexture("glitter.jpg");
        cannonTexture = LoadTexture("glitterPrata.jpg");
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    sideView = true;
    enemiesCount = frenemiesCount / 2;
    gameOver = false;
    score = 0.0f;

    // Reset wall grid
    for (int i = 0; i < sceneWidth; i++)
    {
        for (int j = 0; j < wallHeight; j++)
        {
            wallGrid[i][j] = true;
        }
    }
    WallPosition = Ponto(0, -0.5, sceneDepth / 2);

    srand(time(NULL));
    frenemy = Objeto3D();
    frenemy.LeObjeto("dog.tri");
    frenemies = new Frenemy[frenemiesCount];
    for (int i = 0; i < frenemiesCount; i++)
    {
        Ponto position = Ponto(
            rand() % (sceneWidth - 3),
            -1,
            rand() % ((sceneDepth / 2) - 6) + ((sceneDepth / 2) + 3)
        );
        frenemies[i].position = position;
        frenemies[i].isFriend = i < enemiesCount;
        frenemies[i].isAlive = true;
    }

    CameraRelativePosition = Ponto(0, 2, -5);
    VehicleSize = Ponto(2, 1, 3);
    vehicleMovementDistance = 1.0f;
    CannonSize = Ponto(0.5, 0.5, 2.0);
    VehiclePosition = Ponto(6, 0, 4);
    VehicleAngle = Ponto(0, 0, 0);
    VehicleDirection = Ponto(0, 0, 1);
    CannonRelativePosition = Ponto(0, 0.5, 1);
    CannonAngle = Ponto(0, 0, 0);
    CannonDirection = Ponto(0, 0, 1);
    cannonForce = 25.0f;
    isShooting = false;
    projectileDisplacement = 1.0f;
    Options();
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
    {
        // Termina o programa qdo a tecla ESC for pressionada
        case 27:
            exit(0);
            break;
        case 'p':
        case 'P':
            ModoDeProjecao = !ModoDeProjecao;
            glutPostRedisplay();
            break;
        case 'e':
        case 'E':
            ModoDeExibicao = !ModoDeExibicao;
            init();
            glutPostRedisplay();
            break;
        case 't':
        case 'T':
            sideView = !sideView;
            break;
        case 'v':
        case 'V':
            cout << "Pontuacao atual: " << score << endl;
            cout << "Numero de inimigos restantes: " << enemiesCount << endl;
            Options();
        case 's':
        case 'S':
            MoveVehicle(key);
            break;
        case 'r':
        case 'R':
            init();
            break;
        case 'w':
        case 'W':
            MoveVehicle(key);
            break;
        case 'a':
        case 'A':
            RotateVehicle(key);
            break;
        case 'b':
        case 'B':
            RotateCannon(key);
            break;
        case 'F':
            cannonForce += 1.0f;
            break;
        case 'f':
            cannonForce -= 1.0f;
            break;
        case ' ':
            if (!isShooting) {
                isShooting = true;
                ShootProjectile();
                isShooting = false;
            }
            break;
        default:
            cout << key;
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(700, 700);
	glutCreateWindow("T2 - Carolina Michel Ferreira");

	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrow_keys);
	glutIdleFunc(animate);

	glutMainLoop();
	return 0;
}
