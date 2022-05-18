#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*structure necessary to make the DFS in linear space*/
/*a linked list*/
typedef struct int_list* int_list;

struct int_list{
  int S;
  int_list next;
};


typedef struct node* node;

struct node{
  int Ti;
  int head;
  int sdep;
  node child;
  node father;
  node brother_R;
  node brother_L;
  node slink;
  int nr_of_strings; /*nr of strings this node's path label belongs to*/
  int tree_pos; /*position in the list Tree where this node is*/
  int_list int_list_pos;/*each node will have a pointer to a linked list with
  the information of which strings this node's path label belongs to.*/
};

typedef struct point* point;

struct point
{
  node a; /**< node above */
  node b; /**< node bellow */
  int s; /**< String-Depth */
};

typedef struct g_state* g_state;

struct g_state
{
  char *T; /*concatenation of all strings, w/ "$" terminator between them */
  int *S_sizes; /* list with the sizes of the strings (w/ the terminator)*/
  int *S_beg; /*list with the positions where string i begins in T*/
  int m; /*size of T*/
  int k; /*number of strings*/
  int new_node; /*this variable keeps track of which pos in the Tree we are*/
  struct node *Tree; /*this will be a list with every node */
  int last_int_node; /* last created internal node */
};


/*function that given the point and the character being analyzed, 
returns True or False whether it is possible to descend or not*/
/*if the p is pointing at a node, we need to analyze the first character 
of every edge label of its children, that's why we need this auxiliary node n1, 
to go through the children*/
int descendQ(point p, char c, int from_jump, g_state G){
  node p1;
  int next_char = G->S_beg[p->b->Ti] + p->b->head + p->b->father->sdep + p->s;
  /*if we are at the sentinel, we always descend: reminder that Tree[1] is 
  the root, the only node whose child is the root is the sentinel*/
  if(p->a->child == &G->Tree[1]){p->b = &G->Tree[1];return 1;}
  /*if we are pointing at a node, we need to go through its children:*/
  else if(p->s == 0){
    if(p->a->child == NULL){return 0;}
    else{
      p1 = p->a->child;
      /*going through the child's brothers:*/
      while(p1 != NULL) {
	/*if that child's edge label starts w/ c, we found the way down:*/
        if(G->T[G->S_beg[p1->Ti] + p1->head + p1->father->sdep + p->s] == c){
	  /*save that that child is where we need to descend*/
	  p->b = p1;
	  /* create suffix link if needed */
	  if(G->last_int_node != -1 && !from_jump){
	     G->Tree[G->last_int_node].slink = p->a;
	     G->last_int_node = -1;
	  }
	  return 1;
	}
	else{p1 = p1->brother_R;}
      }
      return 0;
    }
  }
  /*case where the point is in the middle of an edge label. 
  We need to check if the next letter of the edge label is the correct one*/ 
  else if(G->T[next_char] == c){return 1;}
  return 0;
}

void descend(point p, char c, g_state G){
  /*if we are at the sentinel, always go to the root*/
  if(p->a == &G->Tree[0]){
    p->a = &G->Tree[1]; 
    p->b = &G->Tree[1]; 
    p->s = 0;
  }
  else{
    /*we only need to add one to the depth of the node*/
    p->s++;
    /*checking if we reached the end of the edge label, 
    case where we point to the child below*/
    if(p->b->sdep - p->b->father->sdep == p->s){
      p->a = p->b;
      p->s = 0;
    }
  }
}

void suffixJump(point p, g_state G){
  struct point p1;
  /*keep track of where we are on the edge label*/
  int label_size;
  int next_char;
  /*if the node where we are has a suffix link, simply follow it*/
  if (p->a->slink != NULL){p->a = p->a->slink; p->b = p->a; p->s = 0;}
  else{
    /*save the size of the edge label of the node were the point is*/
    label_size = p->a->sdep - p->a->father->sdep;
    /*follow the suffix link of the father*/
    p1.a = p->a->father->slink;
    p1.b = p1.a;
    p1.s = 0;
    /*follow a path down with the edge label of the original node*/
    while(label_size > 0){
      next_char = G->S_beg[p->b->Ti] + p->a->head + p->a->sdep - label_size;
      /*this finds which child we need to descend towards, the third argument 
      "1" states that no suffix links are to be created*/
      descendQ(&p1, G->T[next_char], 1, G);  
      if(label_size >= p1.b->sdep - p1.a->sdep){
        label_size -= p1.b->sdep - p1.a->sdep;
	p1.a = p1.b;
      }
      else{
      	p1.s = label_size; 
      	label_size = 0;
      }
    }
    /*save where we are*/
    p->a = p1.a;
    p->b = p1.b;
    p->s = p1.s;
  }
}

/*p is the poing being used while creating the tree*/
/*pos is the position in current string, of the char being analyzed*/
/*s_ind is the nr of the string being analyzed, needed to define Ti*/
void addleaf(point p, int pos, int s_ind, g_state G){
  node p1;
  int leaf_sdep;
  /*the point is in the middle of an edge, then add internal node, then leaf*/
  if(p->s > 0){
    G->Tree[G->new_node].father = p->a;
    G->Tree[G->new_node].sdep = p->s + p->a->sdep;
    G->Tree[G->new_node].Ti = p->b->Ti;
    G->Tree[G->new_node].head = p->b->head;
    G->Tree[G->new_node].child = p->b;
    G->Tree[G->new_node].tree_pos = G->new_node;
    if(p->b->brother_R != NULL) {
      G->Tree[G->new_node].brother_R = p->b->brother_R;
      p->b->brother_R->brother_L = &G->Tree[G->new_node];
    }
    p->b->father = &G->Tree[G->new_node];
    /*create suffix link, just like in the descendQ*/
    if(G->last_int_node != -1){
      G->Tree[G->last_int_node].slink = &G->Tree[G->new_node];
    }
    G->last_int_node = G->new_node;
    /*case 1: the point is between the father and its child*/
    if(p->b == p->a->child){p->a->child = &G->Tree[G->new_node];}
    /*case 2: the point is between the father and a brother of the child*/
    else{
      G->Tree[G->new_node].brother_L = p->b->brother_L;
      p->b->brother_L->brother_R = &G->Tree[G->new_node];
    }
    G->new_node++;
    /*adding the new leaf*/
    leaf_sdep = G->S_sizes[s_ind] - pos + G->Tree[G->new_node-1].sdep;
    G->Tree[G->new_node].sdep = leaf_sdep;
    G->Tree[G->new_node].Ti = s_ind;
    G->Tree[G->new_node].head = pos - G->Tree[G->new_node-1].sdep;
    G->Tree[G->new_node].father = &G->Tree[G->new_node-1];
    G->Tree[G->new_node].tree_pos = G->new_node;
    G->Tree[G->new_node].brother_L = p->b;
    p->b->brother_R = &G->Tree[G->new_node];
    G->Tree[G->new_node].nr_of_strings = 1;
    G->new_node++;
    p->a = &G->Tree[G->new_node - 2];
    p->b = &G->Tree[G->new_node - 2];
    p->s = 0;
  }
  /*if the point is pointing at a node, add a new child to it*/
  else{
    G->Tree[G->new_node].father = p->a;
    leaf_sdep = G->S_sizes[s_ind] - pos + G->Tree[G->new_node].father->sdep;
    G->Tree[G->new_node].sdep = leaf_sdep;
    G->Tree[G->new_node].Ti = s_ind;
    G->Tree[G->new_node].head = pos - G->Tree[G->new_node].father->sdep;
    G->Tree[G->new_node].tree_pos = G->new_node;
    G->Tree[G->new_node].nr_of_strings = 1;
    /*if the node being pointed at has no children*/
    if(p->a->child == NULL){p->a->child = &G->Tree[G->new_node];}
    /*else,go to its child, find the last brother, insert the new node there*/
    else{
      p1 = p->a->child;
      while (p1->brother_R != NULL){p1 = p1->brother_R;}
      G->Tree[G->new_node].brother_L = p1;
      p1->brother_R = &G->Tree[G->new_node];
    }
    /*time to add suffix links, just like in the descendQ*/
    if(G->last_int_node != -1){
      G->Tree[G->last_int_node].slink = G->Tree[G->new_node].father;
      G->last_int_node = -1;}
    G->new_node++;
  }
}



void buildTree(g_state G){
  int index;
  int i;
  struct point p;
  /*insert sentinel to tree*/
  G->Tree[0].Ti = 0;
  G->Tree[0].sdep = -1;
  G->Tree[0].head = 0;
  G->Tree[0].father = &G->Tree[0];
  G->Tree[0].slink = &G->Tree[0];
  G->Tree[0].tree_pos = 0;
  G->new_node++;
  /*insert the root in the tree*/
  G->Tree[1].Ti = 0;
  G->Tree[1].sdep = 0;
  G->Tree[1].head = 0;
  G->Tree[1].father = &G->Tree[0];
  G->Tree[1].slink = &G->Tree[0];
  G->Tree[1].tree_pos = 1;
  G->Tree[0].child = &G->Tree[1];
  /*define the pointer as starting in the root*/
  p.a = &G->Tree[1];
  p.b = &G->Tree[1];
  p.s = 0;
  G->new_node++;
  /*for each string*/
  for(i = 0; i < G->k; i++){
    /*change the terminator of the string being analyzed*/
    G->T[G->S_beg[i] + G->S_sizes[i] - 1] = '&';
    /*add string i to the tree*/
    for(index = 0; index < G->S_sizes[i]; index ++){
      /*the third argument in the descendQ, 0, 
      means that this function can add suffix links*/
      if (descendQ(&p, G->T[G->S_beg[i] + index], 0, G)){ 
      	descend(&p, G->T[G->S_beg[i] + index], G);
      }
      else{
      	addleaf(&p, index, i, G);  
      	suffixJump(&p, G);
      	index--;
      }
    }
    /*change the terminator back to what is was*/
    G->T[G->S_beg[i] + G->S_sizes[i] - 1] = '$';
  }
}


/* T will be a string that is the concatenation of all k strings,
 with $ between them, $ works as a terminator */
/* this function also creates m, the length of T, which is the sum of 
the lengths off all k strings plus k, because of the k terminator */
/* this function also creates the global var k, which is the nr of strings */ 
void buildT(g_state G){
  char h;
  int i;
  int j;
  int size_of_string;
  /* get number of strings, stored in k */
  h = getchar();
  while( h != '\n' ){
    G->k = (G->k * 10) + (h - '0');
    h = getchar();
  }
  /*initialize the S_sizes and the S_beg:
  S_sizes[i] has the size of string i w/ the terminator and
  S_beg[i] has the position, in T, where string i begins */
  G->S_sizes = realloc(G->S_sizes, G->k * sizeof(int));
  G->S_beg = realloc(G->S_beg, G->k * sizeof(int));
  /* this cycle will analyze the k strings */
  for(i=0;i < G->k; i++){
    /* here we get the size of string i */
    h = getchar();
    size_of_string = 0;
    while( !isspace(h) ){
      size_of_string = (size_of_string * 10) + (h - '0');
      h = getchar();
    }
    /*insert in S_sizes, the +1 is due to the terminator that will be added*/
    G->S_sizes[i] = size_of_string + 1;    
    /*S_beg[i] is the position, in T, where string i begins*/
    G->S_beg[i] = G->m;
    /* now we get the string itself */
    G->T = realloc(G->T,(G->m + size_of_string + 1) * sizeof(char));
    for(j = 0; j < size_of_string; j++){
      h = getchar();
      G->T[G->m + j] = h;
    }
    /* insert the terminator at the end of the individual string */ 
    G->T[G->m + size_of_string] = '$';
    /* m is the sum of the sizes of the strings */
    /* the "+1" is because of the terminator */
    G->m = G->m + size_of_string + 1;
    /* this getchar gets rid of the '\n' character */
    h = getchar();
  }
}

/*merge the two linked lists, the one related to the father and 
the other one to the brother being merged*/
/*the only information that is important is the linked list of the father, the ones of 
the sons can be destroyed in the process */
void merge_sort(int_list father_list,int_list child_list, node n){
  int_list aux;
  /* "brother"'s list starts with a smaller value, then the 2 list switch */
  if(father_list->S > child_list->S){
    aux = child_list;
    child_list = father_list;
    father_list = aux;
    n->father->int_list_pos = father_list;
    n->father->nr_of_strings = n->nr_of_strings;
  }
  /*while there are still elements to join*/
  while(child_list != NULL){
    if (father_list->next != NULL){
      if (father_list->S < child_list->S){
        if(father_list->next->S > child_list->S){
	  aux = child_list->next;
	  child_list->next = father_list->next;
          father_list->next = child_list;
	  child_list = aux;
	  father_list = father_list->next;
	  n->father->nr_of_strings++;
        }
        else if (father_list->next->S == child_list->S){
	  child_list = child_list->next;
        }
        else{father_list = father_list->next;}
      }
      else if(father_list->S == child_list->S){child_list = child_list->next;}
      else{printf("this never happens");}
    }
    else{
      if (child_list->S >father_list->S){
	father_list->next = child_list;
	child_list = child_list->next;
	father_list = father_list->next;
	father_list->next = NULL;
	n->father->nr_of_strings++;
      }
      else{child_list = child_list->next;}
    }
  }
}

/*prints the desired output*/
void reorder_and_print(g_state G, int *s){
  int index;
  for(index = G->k - 2; index > 0; index--){
    if(s[index]>s[index-1]){
      s[index-1] = s[index];
    }
  }
  for(index = 0; index < G->k - 1; index++){
    printf("%d ", s[index]);
  }
  printf("\n");
}

/*Performs the second DFS*/
void find_strings(g_state G){
  int *visited_nodes2;
  int *substring_sizes;
  node n;
  /*node being visited*/
  n = &G->Tree[1];
  /*list containing the info about which nodes have already been analyzed*/
  visited_nodes2 = calloc(G->new_node, sizeof(int));
  /* substring_sizes[i] is the size of the 
  biggest substring common to i+2 strings*/
  substring_sizes = calloc((G->k - 1), sizeof(int));
  while(visited_nodes2[1] == 0){  
    if(visited_nodes2[n->tree_pos] || n->child == NULL){
      if(n->nr_of_strings > 1){
        if(substring_sizes[n->nr_of_strings - 2] < n->sdep){
	  substring_sizes[n->nr_of_strings - 2] = n->sdep;
        }
      }
      /*if this node has a brother, go analyze it */
      if(n->brother_R != NULL){n = n->brother_R;}
      /*if it doesn't, say the father is analyzed*/
      else{
        visited_nodes2[n->father->tree_pos] = 1;
        n = n->father;
      }
    }
    /*otherwise, analyze your child*/
    else{n = n->child;}
  }
  reorder_and_print(G, substring_sizes);
  free(visited_nodes2);
  free(substring_sizes);
}

/* function that does a DFS in the tree and sees, for each node, 
for how many strings that node's path label belongs to */
/*if the node is already analyzed or is a leaf, 
then analyze the brother, if there is no brother, 
merge the list of all the brothers, say the father 
is already analyzed and go to the father */
void DFS(g_state G){
  int *visited_nodes;
  node n;
  struct int_list *linked_lists;
  int new_linked_lists = 0; /*"new_node" for the linked lists*/
  n = &G->Tree[1];
  /*list containing the info about which nodes have already been analyzed*/
  visited_nodes = calloc(G->new_node, sizeof(int));
  /*we will build a linked list for each leaf*/
  linked_lists = calloc(G->new_node, sizeof(struct int_list));
  while(visited_nodes[1] == 0){
    if(visited_nodes[n->tree_pos] || n->child == NULL){
      if(n->child == NULL){
        linked_lists[new_linked_lists].S = n->Ti;
        n->int_list_pos = &linked_lists[new_linked_lists];
        n->nr_of_strings = 1;
        new_linked_lists++;
      }
      if(n->brother_R != NULL){n = n->brother_R;}
      else{
	n = n->father->child;
	/*the linked list of the father starts as the linked list of its child, 
	and is sequentially merged with its brothers*/
	n->father->int_list_pos = n->int_list_pos;
	n->father->nr_of_strings = n->nr_of_strings;
	while(n->brother_R != NULL){
          n = n->brother_R;
	  merge_sort(n->father->int_list_pos, n->int_list_pos, n);
	}
	visited_nodes[n->father->tree_pos] = 1;
	n = n->father;
      }
    }
    else{n = n->child;}
  }
  find_strings(G);
  free(visited_nodes);
  free(linked_lists);
}


int main(){
  /*Create the global variables*/
  struct g_state global_var;
  global_var.new_node = 0;
  global_var.m = 0;
  global_var.k = 0;
  global_var.last_int_node = -1;
  global_var.T = malloc(1 * sizeof(char));
  global_var.S_sizes = malloc(1 * sizeof(int));
  global_var.S_beg = malloc(1 * sizeof(int));
  /*Analyse the input*/
  buildT(&global_var);
  global_var.Tree = calloc((2 * global_var.m + 1), sizeof(struct node));
  /*build the generalized tree*/
  buildTree(&global_var);
  /*DFS on the tree*/
  DFS(&global_var);
  /*free the allocated memory*/
  free(global_var.T);
  free(global_var.S_sizes);
  free(global_var.S_beg);
  free(global_var.Tree);
  return 0;
}
