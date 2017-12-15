

#include "Render.h"
#include <iostream>
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <vector>
#include <array>
#include <cmath>

std::vector < std::array<double, 3>> getCircleSection(double L[3], double M[3], double R[3], int zHeight = 0); // функция получения массива точек отсека окружности, прототип

std::vector < std::array<double, 3>> drawHorizontal(int zOffset = 0){ // рисование горизонтальных граней, часть точек вогнутости соединяются с одной точкой призмы через треугольники, часть с другой, остальные точки соединяются через полигон
	double vtx[9][3] = { {0,0,0 + zOffset},{5,-4,0 + zOffset},{5,3,0 + zOffset},{9,0,0 + zOffset},{13,4,0 + zOffset},{7,6,0 + zOffset},{3,6,0 + zOffset},{3,2,0 + zOffset},{12,1,0 + zOffset}};
	std::vector < std::array<double, 3>> vec;
	std::vector < std::array<double, 3>> concavePoints(getCircleSection(vtx[0], vtx[7], vtx[6], zOffset));
	std::vector < std::array<double, 3>> convexPoints(getCircleSection(vtx[3], vtx[8], vtx[4], zOffset));
	glColor3d(255, 0, 0);
	glBegin(GL_TRIANGLES);
	bool reachedMiddle = false;
	for (std::vector < std::array<double, 3>>::iterator it = concavePoints.begin(); it != concavePoints.end() - 1; ++it) {
		glVertex3d((*it)[0], (*it)[1], (*it)[2]);
		if (concavePoints.end() - 1 - it > (concavePoints.size() / 2))
			glVertex3dv(vtx[1]);
		else if (!reachedMiddle) {
			reachedMiddle = true;
			glVertex3dv(vtx[1]);
			glVertex3dv(vtx[2]);
			--it;
			continue;
		}
		else glVertex3dv(vtx[2]);
		glVertex3d((*(it + 1))[0], (*(it + 1))[1], (*(it + 1))[2]);
	}
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3dv(vtx[6]);
	glVertex3dv(vtx[2]);
	for (std::array<double, 3> vtx : convexPoints) {
		glVertex3d(vtx[0], vtx[1], vtx[2]);
	}
	glVertex3dv(vtx[4]);
	glVertex3dv(vtx[5]);
	glEnd();
	vec.insert(vec.cbegin(), concavePoints.begin(), concavePoints.end());
	vec.push_back({vtx[5][0], vtx[5][1], vtx[5][2]});
	vec.insert(vec.cend(), convexPoints.rbegin(), convexPoints.rend());
	vec.push_back({ vtx[2][0], vtx[2][1], vtx[2][2] });
	vec.push_back({ vtx[1][0], vtx[1][1], vtx[1][2] });
	return vec;
}

void drawSides(std::vector<std::array<double,3>> upperSurface, std::vector<std::array<double, 3>> lowerSurface){ // рисование боковых граней, берутся массивы вершин нижней и верхней граней и соединяются вместе через квадраты
	glColor3d(0, 1, 0);
	glBegin(GL_QUADS);
	for (std::vector<std::array<double, 3>>::const_iterator upperIt = upperSurface.begin(), lowerIt = lowerSurface.begin(); upperIt != upperSurface.end() - 1, lowerIt != lowerSurface.end() - 1; ++upperIt, ++lowerIt) {
		if (upperIt == upperSurface.begin()) {
			std::vector<std::array<double, 3>>::const_iterator lastUpperIt = upperSurface.end() - 1, lastLowerIt = lowerSurface.end() - 1;
			glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
			glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
			glVertex3d((*(lastLowerIt))[0], (*(lastLowerIt))[1], (*(lastLowerIt))[2]);
			glVertex3d((*(lastUpperIt))[0], (*(lastUpperIt))[1], (*(lastUpperIt))[2]);
		}
		glVertex3d((*upperIt)[0], (*upperIt)[1], (*upperIt)[2]);
		glVertex3d((*lowerIt)[0], (*lowerIt)[1], (*lowerIt)[2]);
		glVertex3d((*(lowerIt+1))[0], (*(lowerIt+1))[1], (*(lowerIt+1))[2]);
		glVertex3d((*(upperIt+1))[0], (*(upperIt+1))[1], (*(upperIt+1))[2]);
	}
	glEnd();
}
#define PI 3.14159265358979323846
std::vector < std::array<double, 3>> getCircleSection(double L[3], double M[3], double R[3], int zHeight) { // функция получения массива точек отсека окружности, определение
																											// берутся две точки призмы и одна промежуточная, высчитываются координаты серединных перпендикуляров к отрезкам, соединяющим эти точки
																											// пересечение перпендикуляров является центром окружности
																											// находим координаты остальных промежуточных точек через x = radius*cos(i) + x0, y = radius*sin(i) + y0
	double m1 = M[0] - L[0], m2 = R[0] - M[0], n1 = M[1] - L[1], n2 = R[1] - M[1];
	double x2 = (M[0] + L[0]) / 2, y2 = (M[1] + L[1]) / 2, x3 = (R[0] + M[0]) / 2, y3 = (R[1] + M[1]) / 2;
	double x1, y1;
	x1 = (-m2 * x3 - n2 * y3 + n2 * m1 * x2 / n1 + n2 * y2) / (((n2 * m1) - (m2 *n1)) / n1);
	y1 = (m1 * (x2 - x1) + n1 * y2) / n1;
	double centerVtx[] = { x1, y1, 0 + zHeight };
	std::vector<std::array<double, 3>> vtxVec;
	double r_len = sqrt(pow(L[0] - centerVtx[0], 2) + pow(L[1] - centerVtx[1], 2));
	double left_angle = acos((L[0] - centerVtx[0]) / r_len);
	vtxVec.push_back({ L[0], L[1], 0.0 + zHeight });
	for (double i = left_angle - 0.01; ; i -= 0.01) {
		double x = r_len * cos(i) + centerVtx[0], y = -1 * r_len * sin(i) + centerVtx[1];
		if (sqrt(pow(R[0] - x, 2) + pow(R[1] - y, 2)) <= 0.1) break;
		vtxVec.push_back({ x, y, 0.0 + zHeight });
	}
	vtxVec.push_back({ R[0], R[1], 0.0 + zHeight });
	glColor3d(0, 0, 0);
	glBegin(GL_LINE_STRIP);
	for (std::array<double, 3> vtx : vtxVec) {
		glVertex3d(vtx[0], vtx[1], vtx[2]);
	}
	glEnd(); 
	/*glColor3d(0, 255, 0); // ERASE LATER
	glBegin(GL_LINES);
	glVertex3dv(L);
	glVertex3dv(M);
	glVertex3dv(R);
	glVertex3dv(M);
	glVertex3dv(centerVtx);
	glVertex3d(x2, y2, 0 + zHeight);
	glVertex3dv(centerVtx);
	glVertex3d(x3, y3, 0 + zHeight);
	glEnd(); // MAYBE ERASE LATER */
	return vtxVec;
}


void Render()
{   
	drawSides(drawHorizontal(5), drawHorizontal());
}   

