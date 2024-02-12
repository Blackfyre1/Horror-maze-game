#include "MazeGenerator.h"
#include "RoomGenerator.h"
#include <GL/glut.h>
#include <random>
#include <vector>
#include <stack>
#include <iostream>

std::pair<int, int> operator +(const std::pair<int, int>& x, const std::pair<int, int>& y) {
	return std::make_pair(x.first + y.first, x.second + y.second);
}


Maze::Maze(int x, int y) {
	rows = x;
	columns = y;
	hwall = (int*)malloc((rows + 1) * (columns) * (sizeof(int)));
	vwall = (int*)malloc((rows) * (columns + 1) * (sizeof(int)));
	for (int i = 0;i < rows + 1;i++)
	{
		for (int j = 0;j < columns;j++)
		{
			hwall[i * columns + j] = 1;
		}
	}
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < columns + 1;j++)
		{
			vwall[i * (columns + 1) + j] = 1;
		}
	}
	srand((unsigned int)time(NULL));
	Drill(x/2, y/2);
	printMaze();
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < columns + 1;j++)
		{
			std::cout << vwall[i * (columns + 1) + j];
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	for (int i = 0;i < rows + 1;i++)
	{
		for (int j = 0;j < columns;j++)
		{
			std::cout << hwall[i * columns + j];
		}
		std::cout << "\n";
	}

	for (int i = 0;i < 4;i++)
	{
		pickup[i][0] = rand() % rows;
		pickup[i][0] = (2 * pickup[i][0] + 1) / 2;
		pickup[i][1] = rand() % columns;
		pickup[i][1] = (2 * pickup[i][1] + 1) / 2;
		std::cout << pickup[i][0] << " " << pickup[i][1] << "\n";
	}
}
void Maze::Drill(int ox, int oy)
{
	std::vector <std::vector<bool> > visited(rows,std::vector<bool>(columns,0));
	int VisitedCells = 1;
	std::stack<std::pair<int, int>> m_stack;
	m_stack.push(std::make_pair(ox, oy));
	visited[ox][oy] = 1;
	std::pair <int, int> north = std::make_pair(0, -1);
	std::pair <int, int> south = std::make_pair(0, 1);
	std::pair <int, int> east = std::make_pair(1, 0);
	std::pair <int, int> west = std::make_pair(-1, 0);
	while (VisitedCells < rows * columns)
	{
		std::vector <std::pair<int, int>> choices;
		std::pair <int, int> curr = m_stack.top();
		std::pair <int, int> cn = (curr + north);
		std::pair <int, int> cs = (curr + south);
		std::pair <int, int> ce = (curr + east);
		std::pair <int, int> cw = (curr + west);
		if (cn.second >= 0 && !visited[cn.first][cn.second])
		{
			choices.push_back(cn);
		}
		if (cs.second < rows && !visited[cs.first][cs.second])
		{
			choices.push_back(cs);
		}
		if (cw.first >= 0 && !visited[cw.first][cw.second])
		{
			choices.push_back(cw);
		}
		if (ce.first < columns && !visited[ce.first][ce.second])
		{
			choices.push_back(ce);
		}
		if (choices.size() != 0)
		{
			int ch = rand() % choices.size();
			m_stack.push(choices[ch]);
			if (curr + north == choices[ch])
			{
				hwall[curr.second * columns + curr.first] = 0;
			}
			else if (curr + south == choices[ch])
			{
				hwall[choices[ch].second * columns + choices[ch].first] = 0;
			}
			else if (curr + east == choices[ch])
			{
				vwall[choices[ch].second * (columns+1) + choices[ch].first] = 0;
			}
			else if (curr + west == choices[ch])
			{
				vwall[curr.second * (columns + 1) + curr.first] = 0;
			}
			VisitedCells++;
			visited[choices[ch].first][choices[ch].second] = 1;
		}
		else
		{
			m_stack.pop();
		}
	}
	for (int i = 0;i < std::max(rows,columns);i++)
	{
		int x = rand() % rows+1;
		int y = rand() % columns;
		if (x == 0 || y == 0 || x == rows || y + 1 == columns)
		{
			continue;
		}
		else {
			hwall[(x*columns) + y] = 0;
		}
	}
	for (int i = 0;i < std::max(rows, columns);i++)
	{
		int x = rand() % rows;
		int y = rand() % columns + 1;
		if (x == 0 || y == 0 || x + 1 == rows || y == columns)
		{
			continue;
		}
		else {
			vwall[(x * (columns + 1)) + y] = 0;
		}
	}
}

void Maze::printMaze()
{
	for (int i = 0;i < columns;i++)
	{
		if (hwall[i] == 1)
		{
			std::cout << " _";
		}
		else
		{
			std::cout << "  ";
		}
	}
	std::cout << "\n";
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < columns;j++)
		{
			if (vwall[i*(columns+1) + j] == 1)
			{
				std::cout << "|";
			}
			else
			{
				std::cout << " ";
			}
			if (hwall[(i+1)*columns + j] == 1)
			{
				std::cout << "_";
			}
			else
			{
				std::cout <<" ";
			}
		}
		if (vwall[(i)*(columns+1)] == 1)
		{
			std::cout << "|";
		}
		else
		{
			std::cout << " ";
		}
		std::cout << "\n";
	}
}

void Maze::renderMaze(float size,float thickness)
{
	loadTextures();
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < columns;j++)
		{
			drawFloor3D(i, j, size);
			drawCeiling3D(i, j, size);
		}
	}
	for (int i = 0;i < rows + 1;i++)
	{
		for (int j = 0;j < columns;j++)
		{
			if (hwall[i * columns + j] == 1)
			{
				drawHorizontal3D(j, i, size, thickness);
			}

		}
	}
	for (int i = 0;i < rows;i++)
	{
		for (int j = 0;j < columns + 1;j++)
		{
			if (vwall[i * (columns + 1) + j] == 1)
			{
				drawVertical3D(j, i, size, thickness);
			}

		}
	}
	unloadTextures();
}