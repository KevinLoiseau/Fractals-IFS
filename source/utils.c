#include <stdio.h>
#include <stdlib.h>
#include "../header/utils.h"
#include "../header/geometry.h"


int power(int x, int y) {
	int result = 1;
	int i;
	for(i = 0 ; i<y ; i++) {
		result*= x;
	}	
	return result;
}

int min(int a, int b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}
