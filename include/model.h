#include "globalVars.h"


/* Model Functions */

// k: Z, i: X, j: Y
struct node getNode(int k, int i, int j, struct node* model);
void setNode(int k, int i, int j, struct node* model, struct node val);

void initModel(int print_progress, struct node* model);
void buildModel(int print_progress);
