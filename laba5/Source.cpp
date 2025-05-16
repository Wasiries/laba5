#include <windows.h>
#include <gl\GL.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <string>
#include <algorithm>
#define _USE_MATH_DEFINES
#define PI 3.14159265

using namespace std;

float const xmax = 10, xmin = -1, ymax = 10, ymin = -1, delta = 0.2, eps = 0.05, EPS = 1e-9;
int width = 800, height = 800, num = 0, vis = 0, invis = 0, score = 0;
unsigned int timer, currentTime;
bool paused, generatable, scoreChanged, drawable, answer;


double random(double min_, double max_) {
    return min_ + double(rand()) * (max_ - min_) / double(RAND_MAX);
}

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    void show() {
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glEnd();
    }
};

Point convert(int x, int y) {
    return Point(double(xmin) + double(x) * double(xmax - xmin) / double(width),
        double(ymax) - double(y) * double(ymax - ymin) / double(height));
}

struct Edge {
    Point a, b;
    Edge(Point a = Point(), Point b = Point()) : a(a), b(b) {}
    Edge normal() {
        return Edge(a, Point(a.x - dify(), a.y + difx()));
    }
    double len() {
        return pow((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y), 0.5);
    }
    double difx() {
        return b.x - a.x;
    }
    double dify() {
        return b.y - a.y;
    }
};


double pseu(Edge first, Edge second) {
    return first.difx() * second.dify() - first.dify() * second.difx();
}
double scal(Edge first, Edge second) {
    return first.difx() * second.difx() + first.dify() * second.dify();
}


vector<Point> polygon = vector<Point>();
vector<Point> invisible = vector<Point>();
vector<Point> visible = vector<Point>();

void showPolygon() {
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < num; i++) {
        glVertex2f(polygon[i - 1].x, polygon[i - 1].y);
        glVertex2f(polygon[i].x, polygon[i].y);
    }
    glEnd();
    glBegin(GL_POINTS);
    for (int i = 0; i < num; i++) {
        glVertex2f(polygon[i].x, polygon[i].y);
    }
    glEnd();
}

void showInvisible() {
    glColor3f(1.0, 0.3, 0.3);
    glBegin(GL_LINES);
    for (int i = 0; i < invis / 2; i++) {
        glVertex2f(invisible[i * 2].x, invisible[i * 2].y);
        glVertex2f(invisible[i * 2 + 1].x, invisible[i * 2 + 1].y);
    }
    glEnd();
    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0);
    if (invis > 0) {
        glVertex2f(invisible[invis - 1].x, invisible[invis - 1].y);
    }
    glColor3f(1.0, 0.3, 0.3);
    for (int i = 0; i < invis; i++) {
        glVertex2f(invisible[i].x, invisible[i].y);
    }
    glEnd();
}

void showVisible() {
    glColor3f(0.3, 1.0, 0.4);
    glBegin(GL_LINES);
    for (int i = 0; i < vis / 2; i++) {
        glVertex2f(visible[i * 2].x, visible[i * 2].y);
        glVertex2f(visible[i * 2 + 1].x, visible[i * 2 + 1].y);
    }
    glEnd();
    glBegin(GL_POINTS);
    for (int i = 0; i < vis; i++) {
        glVertex2f(visible[i].x, visible[i].y);
    }
    glEnd();
}

void showBorder() {
    float d = 0.01;
    glColor3f(0.5, 0.5, 0.5);
    glLineWidth(2);
    glLineStipple(1, 0x7FE);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINES);
    for (int i = ceil(xmin + d); i <= floor(xmax - d); i++) {
        glVertex2f(i, ymin + delta);
        glVertex2f(i, ymax - delta);
    }
    for (int i = ceil(ymin + d); i <= floor(ymax - d); i++) {
        glVertex2f(xmin + delta, i);
        glVertex2f(xmax - delta, i);
    }
    glEnd();
    glColor3f(1.0, 1.0, 1.0);
    glDisable(GL_LINE_STIPPLE);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(xmin + delta, 0.0);
    glVertex2f(xmax - delta, 0.0);
    glVertex2f(0.0, ymin + delta);
    glVertex2f(0.0, ymax - delta);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(-delta, ymax - 3 * delta);
    glVertex2f(0, ymax - delta);
    glVertex2f(delta, ymax - 3 * delta);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(xmax - 3 * delta, -delta);
    glVertex2f(xmax - delta, 0);
    glVertex2f(xmax - 3 * delta, delta);
    glEnd();
}

void showScore(string text, double beginX, double beginY) {
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2d(beginX, beginY);
    for (int i = 0; i < text.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
}

void showTime(string text, double beginX, double beginY) {
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos2d(beginX, beginY);
    for (int i = 0; i < text.length(); i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    showBorder();
    showPolygon();
    showInvisible();
    showVisible();
    showScore("score: " + to_string(score), 0.0, 9.0);
    showTime("time: " + to_string(timer / 1000), 0.0, 0.0);
    glFlush();
    glutSwapBuffers();
}

void generation() {
    double maxstep = 2.0;
    double minstep = 1.0;
    double maxangle = PI;
    double minangle = PI * 0.125;
    double angle = 0.0;
    double dangle = 0.0;
    double step = 0.0;
    polygon = vector<Point>();
    Point first = Point(random(2.0, 7.0), random(2.0, 7.0));
    polygon.push_back(first);
    dangle = random(0.0, PI);
    step = random(minstep, maxstep);
    Point second = Point(first.x + step * cos(dangle), first.y + step * sin(dangle));
    angle += dangle;
    polygon.push_back(second);
    num = 2;
    Point prev1 = second, prev2 = first;
    bool finished = false;
    while (!finished) {
        dangle = random(minangle, maxangle);
        step = random(minstep, maxstep);
        angle += dangle;
        double x = prev1.x + step * cos(angle);
        double y = prev1.y + step * sin(angle);
        Point point = Point(x, y);
        if (pseu(Edge(first, second), Edge(first, point)) >= 0.0) {
            finished = true;
            polygon.push_back(first);
        }
        else {
            polygon.push_back(point);
        }
        num++;
        prev2 = prev1;
        prev1 = point;
    }
}

bool isvisible(Point begin, Point end, Point* resultBegin, Point* resultEnd) {
    double b = 0.0;
    double e = 1.0;
    Edge direction = Edge(begin, end);  
    int size = num;
    for (int i = 0; i < size; i++) {
        Edge side = Edge(polygon[i], polygon[(i + 1) % size]);
        Edge normal = side.normal();
        double sc1 = scal(normal, direction);
        double sc2 = scal(normal, Edge(side.a, begin));
        double t;
        if (sc1 > 0.0) {
            t = -1 * sc2 / sc1;
            b = max(b, t);
        }
        else {
            if (sc1 < 0.0) {
                t = -1 * sc2 / sc1;
                e = min(e, t);
            }
            else {
                if (sc2 < 0.0) {
                    return false;
                }
            }
        }
    }
    if (e < b) {
        return false;
    }
    *resultBegin = Point(begin.x + direction.difx() * b, begin.y + direction.dify() * b);
    *resultEnd = Point(begin.x + direction.difx() * e, begin.y + direction.dify() * e);
    return true;
}

void addPoint(Point point) {
    invisible.push_back(point);
    invis++;
    answer = false;
    if (invis % 2 == 0) {
        Point prev = invisible[invis - 2];
        Point temp1, temp2;
        answer = isvisible(prev, point, &temp1, &temp2);
        if (answer) {
            visible.push_back(temp1);
            visible.push_back(temp2);
            vis += 2;
        }
    }
}

void clear() {
    if (!paused) {
        polygon = vector<Point>();
        invisible = vector<Point>();
        visible = vector<Point>();
        num = 0;
        vis = 0;
        invis = 0;
    }
}

void gameInit() {
    timer = 0;
    score = 0;
    polygon = vector<Point>();
    invisible = vector<Point>();
    visible = vector<Point>();
    num = 0;
    vis = 0;
    invis = 0;
    paused = false;
    generatable = true;
    drawable = true;
    scoreChanged = false;
    answer = false;
}

void gameContinue() {
    currentTime = clock();
}

void showResults() {
    if (!scoreChanged) {
        if (answer) {
            score += 100;
        }
        scoreChanged = true;
    }
}

void gameplay() {
    if (!paused) {
        timer += clock() - currentTime;
        currentTime = clock();
    }
    display();
}

void timerFunc(int) {
    if (!paused) {
        gameplay();
    }
    glutTimerFunc(30, timerFunc, 0);
}

void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            addPoint(convert(x, y));
        }
        if (button == GLUT_RIGHT_BUTTON) {
            clear();
        }
        display();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'q':
        paused = true;
        return;
    case 'w':
        if (!paused and generatable) {
            clear();
            generation();
        }
        return;
    case 'e':
        if (paused) {
            paused = false;
            gameContinue();
        }
        return;
    case 'r':
        gameInit();
        return;
    }
}

void init(void) {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(50, 10);
    glutCreateWindow("Okna 11");
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(xmin, xmax, ymin, ymax);
    glPointSize(8);
    glLineWidth(3);
    glClearColor(0.2, 0.3, 0.9, 1.0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timerFunc, 0);
    glutMainLoop();
    return 0;
}