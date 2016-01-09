#ifndef H_DRAGON
#define H_DRAGON

double (*w)[6];
int nbFonctions;
segment* morceau;
segment* fractal;

void applyAffineFonctions(segment* seg, double a, double b, double c, double d, double e, double f);
void applyAffineFonctionPoint(point* point, double a, double b, double c, double d, double e, double f);
#endif
