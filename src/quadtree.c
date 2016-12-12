#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>

typedef struct { // Headerul unui fisier BMP
	unsigned short signature;
	unsigned int size;
	unsigned int reserved;
	unsigned int offset;
} Header;

typedef struct { // Infoheaderul unui fisier BMP
	unsigned int size;
	int width, height;
	unsigned short planes;
	unsigned short bits;
	unsigned int compression;
	unsigned int imagesize;
	int xresolution, yresolution;
	unsigned int ncolours;
	unsigned int importantcolours;
} Infoheader;

typedef struct { // Un pixel din matricea de pixeli
	unsigned char r, g, b, reserved;
} Colors;

typedef struct quadtree { // Arborele folosit
	unsigned char r, g, b, rs;
	struct quadtree *top_left;
	struct quadtree *top_right;
	struct quadtree *bottom_left;
	struct quadtree *bottom_right;
	int area;
	int indice;
} *Quadtree;

typedef struct QuadtreeNode {
	unsigned char blue, green, red, reserved;
	uint32_t area;
	int32_t top_left, top_right;
	int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;

Quadtree initQuadtree(char r, char g, char b,
					  int area, char rs) { // Initializarea unui nod
	Quadtree tree;
	tree = (Quadtree) malloc(sizeof(struct quadtree));
	tree->r = r;
	tree->g = g;
	tree->b = b;
	tree->area = area * area;
	tree->rs = rs;
	tree->top_left = NULL;
	tree->top_right = NULL;
	tree->bottom_right = NULL;
	tree->bottom_left = NULL;
	return tree;
}

int isLeaf(Colors **v, int size, int i, int j);

Quadtree buildQuadtree(Colors **v, int size, int i, int j) { // Construirea arborelui din matricea de pixeli
	Quadtree tree = NULL;
	if (isLeaf(v, size, i, j) == 1) { // Daca s-a gasit o frunza
		tree = initQuadtree(v[i][j].r, v[i][j].g, v[i][j].b, size, v[i][j].reserved);
		return tree;
	}
	else { // Daca nu, apelare recursiva in submatrici
		tree = initQuadtree(0, 0, 0, size, 0);
		tree->top_left = buildQuadtree(v, size / 2, i + size / 2, j);
		tree->top_right = buildQuadtree(v, size / 2, i, j);
		tree->bottom_right = buildQuadtree(v, size / 2, i, j + size / 2);
		tree->bottom_left = buildQuadtree(v, size / 2, i + size / 2, j + size / 2);
	}
	return tree;

}

Quadtree buildQuadtree3(Colors **v, int size, int i, int j) {
	Quadtree tree = NULL;
	if (isLeaf(v, size, i, j) == 1) {
		tree = initQuadtree(v[i][j].r, v[i][j].g, v[i][j].b, size, v[i][j].reserved);
		return tree;
	}
	else {
		tree = initQuadtree(0, 0, 0, size, 0);
		tree->top_left = buildQuadtree(v, size / 2, i, j);
		tree->top_right = buildQuadtree(v, size / 2, i, j + size / 2);
		tree->bottom_right = buildQuadtree(v, size / 2, i + size / 2, j + size / 2);
		tree->bottom_left = buildQuadtree(v, size / 2, i + size / 2, j);
	}
	return tree;
}

Header ReadHeader(FILE *a) { // Citirea unui Header
	Header header;
	fread(&header.signature, 2, 1, a);
	fread(&header.size, 4, 1, a);
	fread(&header.reserved, 4, 1, a);
	fread(&header.offset, 4, 1, a);
	return header;
}

Infoheader ReadInfoheader(FILE *a) { // Citirea unui Infoheader
	Infoheader infoheader;
	fread(&infoheader, sizeof(Infoheader), 1, a);
	return infoheader;
}

void ReadColors(FILE *a, Colors **b, int height, int width) { // Citirea matricei de pixeli
	int i, j;
	for (i = height - 1; i >= 0; i--)
		for (j = 0; j < width; j++) {
			fread(&b[i][j].b, sizeof(unsigned char), 1, a);
			fread(&b[i][j].g, sizeof(unsigned char), 1, a);
			fread(&b[i][j].r, sizeof(unsigned char), 1, a);
			fread(&b[i][j].reserved, sizeof(unsigned char), 1, a);
		}
}

void ReadColorss(FILE *a, Colors **b, int height, int width) {
	int i, j;
	for (i = height - 1; i >= 0; i--) {
		for (j = 0; j < width; j++) {
			fread(&b[i][j].b, sizeof(unsigned char), 1, a);
			fread(&b[i][j].g, sizeof(unsigned char), 1, a);
			fread(&b[i][j].r, sizeof(unsigned char), 1, a);
			fread(&b[i][j].reserved, sizeof(unsigned char), 1, a);
		}
	}
}

int isLeaf(Colors **v, int size, int i, int j) { // Verifica daca o submatrice este frunza
	// (contine doar o singura culoare)
	int t, k;
	for (t = i; t < size + i; t++) {
		for (k = j; k < size + j; k++) {
			if (v[t][k].r != v[i][j].r ||
				v[t][k].g != v[i][j].g ||
				v[t][k].b != v[i][j].b)
				return 0;
		}
	}
	return 1;
}

void fillMatrix(Colors **v, Quadtree tree, int i, int j) { // Umple o submatrice cu o culoare din arbore
	int t, k;
	for (t = i; t < sqrt((double) tree->area) + i; t++) {
		for (k = j; k < sqrt((double) tree->area) + j; k++) {
			v[t][k].r = tree->r;
			v[t][k].g = tree->g;
			v[t][k].b = tree->b;
			v[t][k].reserved = tree->rs;
		}
	}
}

int NoLeafs(Colors **v, int size, int i, int j) { // Numara frunzele
	if (isLeaf(v, size, i, j) == 1)
		return 1;
	return NoLeafs(v, size / 2, i, j) + NoLeafs(v, size / 2, i, j + size / 2) +
		   NoLeafs(v, size / 2, i + size / 2, j) + NoLeafs(v, size / 2, i + size / 2, j + size / 2);
}

Quadtree freeTree(Quadtree tree) { // Elibereaza memoria unui arbore
	if (tree == NULL)
		return tree;
	tree->top_left = freeTree(tree->top_left);
	tree->top_right = freeTree(tree->top_right);
	tree->bottom_right = freeTree(tree->bottom_right);
	tree->bottom_left = freeTree(tree->bottom_left);
	free(tree);
	return NULL;
}

int max(int a, int b) { // Maximul dintre 2 numere
	if (a >= b)
		return a;
	else
		return b;
}

int inaltime(Quadtree tree) { // Inaltimea unui arbore
	if (tree == NULL)
		return 0;
	else {
		int tlheight = inaltime(tree->top_left);
		int trheight = inaltime(tree->top_right);
		int brheight = inaltime(tree->bottom_right);
		int blheight = inaltime(tree->bottom_left);
		return max(max(tlheight, trheight), max(brheight, blheight)) + 1;
	}
}

int printGivenLevel(Quadtree tree, int level) { // Printeaza un nivel dintr-un arbore
	static int i = 0;
	if (tree == NULL)
		return 0;
	if (level == 1) {
		tree->indice = i;
		i++;
	}
	else if (level > 1) {
		printGivenLevel(tree->top_left, level - 1);
		printGivenLevel(tree->top_right, level - 1);
		printGivenLevel(tree->bottom_right, level - 1);
		printGivenLevel(tree->bottom_left, level - 1);
	}
	return i;
}

void createVector(Quadtree tree, int level, QuadtreeNode *v) { //Functia de creeare a vectorului de compresie
	static int i = 0;
	if (tree == NULL)
		return;
	if (level == 1) {
		v[i].blue = tree->b;
		v[i].green = tree->g;
		v[i].red = tree->r;
		v[i].reserved = tree->rs;
		v[i].area = tree->area;
		if (tree->top_left == NULL) { // Daca e frunza
			v[i].top_left = -1;
			v[i].top_right = -1;
			v[i].bottom_right = -1;
			v[i].bottom_left = -1;
		}
		else { // Daca nu este frunza
			v[i].top_left = tree->top_left->indice;
			v[i].top_right = tree->top_right->indice;
			v[i].bottom_left = tree->bottom_left->indice;
			v[i].bottom_right = tree->bottom_right->indice;
		}
		i++;

	}
	else if (level > 1) {
		createVector(tree->top_left, level - 1, v);
		createVector(tree->top_right, level - 1, v);
		createVector(tree->bottom_right, level - 1, v);
		createVector(tree->bottom_left, level - 1, v);
	}
}

int printLevelOrder(Quadtree tree, QuadtreeNode **v) { //Parcurgerea propriu-zisa pe nivele
	int h = inaltime(tree);
	int i, nr_noduri;
	for (i = 1; i <= h; i++) {
		nr_noduri = printGivenLevel(tree, i);
	}
	*v = (QuadtreeNode *) malloc(nr_noduri * sizeof(QuadtreeNode));
	for (i = 1; i <= h; i++)
		createVector(tree, i, *v);
	return nr_noduri;
}

void Cerinta1(FILE *out, Header header, Infoheader infoheader,
			  int nr_frunze, int nr_noduri, QuadtreeNode *v) { // Scrierea in fisierul de compresie
	fwrite(&header.signature, 2, 1, out);
	fwrite(&header.size, 4, 1, out);
	fwrite(&header.reserved, 4, 1, out);
	fwrite(&header.offset, 4, 1, out);
	fwrite(&infoheader, sizeof(Infoheader), 1, out);
	fwrite(&nr_frunze, sizeof(int), 1, out);
	fwrite(&nr_noduri, sizeof(int), 1, out);
	fwrite(v, sizeof(QuadtreeNode), nr_noduri, out);
}

Quadtree buildQuadtree2(QuadtreeNode *v, int i) {
	Quadtree tree = NULL;
	if (v[i].top_left == -1) {
		tree = initQuadtree(v[i].red, v[i].green, v[i].blue, sqrt((double) v[i].area),
							v[i].reserved);
		return tree;
	}
	else {
		tree = initQuadtree(0, 0, 0, sqrt((double) v[i].area), 0);
		tree->top_left = buildQuadtree2(v, v[i].top_right);
		tree->top_right = buildQuadtree2(v, v[i].bottom_right);
		tree->bottom_left = buildQuadtree2(v, v[i].top_left);
		tree->bottom_right = buildQuadtree2(v, v[i].bottom_left);
	}
	return tree;
}

int citireNrculori(FILE *compress) { // Citirea nr de culori
	int nr_culori;
	fread(&nr_culori, sizeof(uint32_t), 1, compress);
	return nr_culori;
}

int citireNrnoduri(FILE *compress) { // Citirea nr de noduri
	int nr_noduri;
	fread(&nr_noduri, sizeof(int), 1, compress);
	return nr_noduri;
}

void matrix2(Colors **v, Quadtree tree, int i, int j) { // Construirea matricei din arbore
	if (tree->top_left == NULL) {
		fillMatrix(v, tree, i, j);
	}
	else {
		matrix2(v, tree->top_left, i, j);
		matrix2(v, tree->top_right, i, j + sqrt((double) tree->area) / 2);
		matrix2(v, tree->bottom_right, i + sqrt((double) tree->area) / 2,
				j + sqrt((double) tree->area) / 2);
		matrix2(v, tree->bottom_left, i + sqrt((double) tree->area) / 2, j);
	}
}

void Cerinta2(FILE *out, Header header, Infoheader infoheader,
			  Colors **v, int size, int nr_culori) { // Scrierea unui fisier BMP
	fwrite(&header.signature, 2, 1, out);
	fwrite(&header.size, 4, 1, out);
	fwrite(&header.reserved, 4, 1, out);
	fwrite(&header.offset, 4, 1, out);
	fwrite(&infoheader, sizeof(Infoheader), 1, out);
	int i, j;
	for (i = size - 1; i >= 0; i--) {
		for (j = 0; j < size; j++) {
			fwrite(&v[i][j].b, sizeof(unsigned char), 1, out);
			fwrite(&v[i][j].g, sizeof(unsigned char), 1, out);
			fwrite(&v[i][j].r, sizeof(unsigned char), 1, out);
			fwrite(&v[i][j].reserved, sizeof(unsigned char), 1, out);
		}
	}
}

Quadtree rotate0(Quadtree tree) { // Prima rotatie
	if (tree == NULL)
		return NULL;
	else {
		Quadtree top_left = tree->top_left;
		Quadtree top_right = tree->top_right;
		Quadtree bottom_right = tree->bottom_right;
		Quadtree bottom_left = tree->bottom_left;
		tree->top_left = top_right;
		tree->top_right = bottom_right;
		tree->bottom_right = bottom_left;
		tree->bottom_left = top_left;
		tree->top_left = rotate0(tree->top_left);
		tree->top_right = rotate0(tree->top_right);
		tree->bottom_right = rotate0(tree->bottom_right);
		tree->bottom_left = rotate0(tree->bottom_left);
	}
	return tree;
}

Quadtree rotate1(Quadtree tree) { // A doua rotatie
	if (tree == NULL)
		return NULL;
	else {
		Quadtree top_left = tree->top_left;
		Quadtree top_right = tree->top_right;
		Quadtree bottom_right = tree->bottom_right;
		Quadtree bottom_left = tree->bottom_left;
		tree->top_left = bottom_right;
		tree->top_right = bottom_left;
		tree->bottom_right = top_left;
		tree->bottom_left = top_right;
		tree->top_left = rotate1(tree->top_left);
		tree->top_right = rotate1(tree->top_right);
		tree->bottom_right = rotate1(tree->bottom_right);
		tree->bottom_left = rotate1(tree->bottom_left);
	}
	return tree;
}

Quadtree rotate2(Quadtree tree) { // A treia rotatie
	if (tree == NULL)
		return NULL;
	else {
		Quadtree top_left = tree->top_left;
		Quadtree top_right = tree->top_right;
		Quadtree bottom_right = tree->bottom_right;
		Quadtree bottom_left = tree->bottom_left;
		tree->top_left = bottom_left;
		tree->top_right = top_left;
		tree->bottom_right = top_right;
		tree->bottom_left = bottom_right;
		tree->top_left = rotate2(tree->top_left);
		tree->top_right = rotate2(tree->top_right);
		tree->bottom_right = rotate2(tree->bottom_right);
		tree->bottom_left = rotate2(tree->bottom_left);
	}
	return tree;
}

int hasColor(Quadtree p, int r1, int g1, int b1, int r2, int g2,
			 int b2, int nr) { //Verifica daca un arbore contine 2 culori
	static int true1 = 0;
	static int true2 = 0;
	if (nr == 1) {
		if (true1 != 0) true1 = 0;
		if (true2 != 0) true2 = 0;
	}
	if (p == NULL)
		return 0;
	if (p->r == r1 && p->g == g1 && p->b == b1)
		true1 = 1;
	if (p->r == r2 && p->g == g2 && p->b == b2)
		true2 = 1;
	else {
		hasColor(p->top_right, r1, g1, b1, r2, g2, b2, nr + 1);
		hasColor(p->top_left, r1, g1, b1, r2, g2, b2, nr + 1);
		hasColor(p->bottom_right, r1, g1, b1, r2, g2, b2, nr + 1);
		hasColor(p->bottom_left, r1, g1, b1, r2, g2, b2, nr + 1);
	}
	return (true1 == 1 && true2 == 1);
}

Quadtree stramos(Quadtree tree, int r1, int g1, int b1, int r2, int g2,
				 int b2, int nr) { // Stramosul comun este ultimul nod care contine ambele culori
	if (hasColor(tree->top_right, r1, g1, b1, r2, g2, b2, nr) == 1) {
		return stramos(tree->top_right, r1, g1, b1, r2, g2, b2, nr);
	}
	else if (hasColor(tree->top_left, r1, g1, b1, r2, g2, b2, nr) == 1) {
		return stramos(tree->top_left, r1, g1, b1, r2, g2, b2, nr);
	}
	else if (hasColor(tree->bottom_right, r1, g1, b1, r2, g2, b2, nr) == 1) {
		return stramos(tree->bottom_right, r1, g1, b1, r2, g2, b2, nr);
	}
	else if (hasColor(tree->bottom_left, r1, g1, b1, r2, g2, b2, nr) == 1) {
		return stramos(tree->bottom_left, r1, g1, b1, r2, g2, b2, nr);
	}
	return tree;
}

int main(int argc, char *argv[]) {
	if (strcmp(argv[1], "-c") == 0) { // Task 1
		FILE *bmp = fopen(argv[2], "rb");
		FILE *golire = fopen(argv[3], "wb");
		fclose(golire);
		FILE *out = fopen(argv[3], "wb");
		Header header;
		Infoheader infoheader;
		header = ReadHeader(bmp);
		infoheader = ReadInfoheader(bmp);
		int size = infoheader.height;
		Colors **colors = (Colors **) malloc(size * sizeof(Colors *));
		int i;
		for (i = 0; i < size; i++)
			colors[i] = (Colors *) malloc(size * sizeof(Colors));
		ReadColors(bmp, colors, infoheader.height, infoheader.width); // Matricea de pixeli
		int nr_frunze = NoLeafs(colors, size, 0, 0);
		Quadtree tree = NULL;
		tree = buildQuadtree(colors, size, 0, 0); // Construirea arborelui
		QuadtreeNode *v;
		int nr_noduri = printLevelOrder(tree, &v); // Construirea vectorului de compresie
		Cerinta1(out, header, infoheader, nr_frunze, nr_noduri, v); // Scrierea in fisier
		for (i = 0; i < size; i++) // Eliberarea de memorie
			free(colors[i]);
		free(v);
		free(colors);
		tree = freeTree(tree);
		fclose(bmp);
		fclose(out);
		return 0;
	}
	else if (strcmp(argv[1], "-d") == 0) { // Task 2
		FILE *compress = fopen(argv[2], "rb");
		FILE *golire = fopen(argv[3], "wb");
		fclose(golire);
		FILE *out = fopen(argv[3], "wb");
		Header header;
		Infoheader infoheader;
		header = ReadHeader(compress);
		infoheader = ReadInfoheader(compress);
		int nr_culori = citireNrculori(compress);
		int nr_noduri = citireNrnoduri(compress);
		QuadtreeNode *v;
		v = (QuadtreeNode *) malloc(nr_noduri * sizeof(QuadtreeNode));
		int i;
		fread(v, sizeof(QuadtreeNode), nr_noduri, compress); // Citirea vectorului
		Quadtree tree = NULL;
		tree = buildQuadtree2(v, 0); // Crearea arborelui
		Colors **matrix = (Colors **) malloc(sqrt((double) tree->area) *
											 sizeof(Colors *));
		for (i = 0; i < sqrt((double) tree->area); i++)
			matrix[i] = (Colors *) malloc(sqrt((double) tree->area) *
										  sizeof(Colors));
		matrix2(matrix, tree, 0, 0); // Crearea matricei de pixeli din arbore
		Cerinta2(out, header, infoheader, matrix, sqrt((double) tree->area), nr_culori); // Scrierea in fisier
		for (i = 0; i < sqrt((double) tree->area); i++) // Eliberarea de memorie
			free(matrix[i]);
		free(matrix);
		free(v);
		tree = freeTree(tree);
		fclose(compress);
		fclose(out);
		return 0;
	}
	else if (strcmp(argv[1], "-r") == 0) { // Task 3
		int nr_rotatii = atoi(argv[2]);
		FILE *photo = fopen(argv[3], "rb");
		FILE *golire = fopen(argv[4], "wb");
		fclose(golire);
		FILE *out = fopen(argv[4], "wb");
		Header header;
		Infoheader infoheader;
		header = ReadHeader(photo);
		infoheader = ReadInfoheader(photo);
		int size = infoheader.height;
		Colors **colors = (Colors **) malloc(size * sizeof(Colors *));
		int i;
		for (i = 0; i < size; i++)
			colors[i] = (Colors *) malloc(size * sizeof(Colors));
		ReadColorss(photo, colors, infoheader.height, infoheader.width); // Citirea matricei de pixeli
		Quadtree tree = NULL;
		tree = buildQuadtree(colors, size, 0, 0); // Construirea arborelui
		if (nr_rotatii % 4 == 0) { // Rotirea in functie de numarul de rotatii
			tree = rotate0(tree);
		}
		else if (nr_rotatii % 4 == 1) {
			tree = rotate1(tree);
		}
		else if (nr_rotatii % 4 == 2) {
			tree = rotate2(tree);
		}
		matrix2(colors, tree, 0, 0); // Reconstructia matricei de pixeli
		Cerinta2(out, header, infoheader, colors, sqrt((double) tree->area), 0); // Scrierea in fisier
		for (i = 0; i < size; i++)
			free(colors[i]);
		free(colors);
		tree = freeTree(tree);
		fclose(photo);
		fclose(out);
		return 0;
	}
	else if (strcmp(argv[1], "-b") == 0) { // Bonus
		int r1 = atoi(argv[2]);
		int g1 = atoi(argv[3]);
		int b1 = atoi(argv[4]);
		int r2 = atoi(argv[5]);
		int g2 = atoi(argv[6]);
		int b2 = atoi(argv[7]);
		FILE *photo = fopen(argv[8], "rb");
		FILE *golire = fopen(argv[9], "wb");
		fclose(golire);
		FILE *out = fopen(argv[9], "wb");
		Header header;
		Infoheader infoheader;
		header = ReadHeader(photo);
		infoheader = ReadInfoheader(photo);
		int size = infoheader.height;
		Colors **colors = (Colors **) malloc(size * sizeof(Colors *));
		int i;
		for (i = 0; i < size; i++)
			colors[i] = (Colors *) malloc(size * sizeof(Colors));
		ReadColorss(photo, colors, infoheader.height, infoheader.width); // Citirea matricei de pixeli
		Quadtree tree = NULL;
		tree = buildQuadtree(colors, size, 0, 0); // Crearea arborelui
		tree = rotate0(tree);
		Quadtree stramoss = stramos(tree, r1, g1, b1, r2, g2, b2, 1); // Aflarea stramosului(va fi un arbore)
		matrix2(colors, stramoss, 0, 0);
		infoheader.width = sqrt((double) stramoss->area);
		infoheader.height = sqrt((double) stramoss->area);
		Cerinta2(out, header, infoheader, colors, sqrt((double) stramoss->area), 0); // Scrierea in fisier a 
		// subimaginii corespunzatoare stramosului
		for (i = 0; i < size; i++) // Eliberarea de memorie
			free(colors[i]);
		free(colors);
		tree = freeTree(tree);
		fclose(photo);
		fclose(out);
		return 0;

	}
	return 0;
}
