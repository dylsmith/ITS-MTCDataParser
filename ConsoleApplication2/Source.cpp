#include "stdafx.h"
#include "QuickParser.h"
#include <iostream>
#include "Header.h"
using namespace std;

void func()
{
	QuickParser Parser = QuickParser("D:\\Farzad\\logistic\\freight\\TripsTrkAMC.csv");
	
	int **values = new int*[4];
	for (int i = 0; i < 1455; i++) {
		values[i] = new int[1455];
	}
	for (int i = 1; i < 1455; i++){
		for (int j = 1; j < 1455; j++)
		{
			values[i][j] = Parser.parseInt();
		}
	}
	for (int i = 1; i < 1455; i++){
		for (int j = 1; j < 1455; j++)
		{
			cout << values[i][j] << " ";
		}
		cout << endl;
	}
}