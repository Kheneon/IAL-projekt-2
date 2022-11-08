/*
 * Binárny vyhľadávací strom — iteratívna varianta
 *
 * S využitím dátových typov zo súboru btree.h, zásobníkov zo súborov stack.h a
 * stack.c a pripravených kostier funkcií implementujte binárny vyhľadávací
 * strom bez použitia rekurzie.
 */

#include "../btree.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>

/*
stack_bst_t *stack;
stack = malloc(sizeof(stack_bst_t));
stack_bst_init(stack);
*/

/*
 * Inicializácia stromu.
 *
 * Užívateľ musí zaistiť, že incializácia sa nebude opakovane volať nad
 * inicializovaným stromom. V opačnom prípade môže dôjsť k úniku pamäte (memory
 * leak). Keďže neinicializovaný ukazovateľ má nedefinovanú hodnotu, nie je
 * možné toto detegovať vo funkcii.
 */
void bst_init(bst_node_t **tree) {
  *tree = NULL;
}

/*
 * Nájdenie uzlu v strome.
 *
 * V prípade úspechu vráti funkcia hodnotu true a do premennej value zapíše
 * hodnotu daného uzlu. V opačnom prípade funckia vráti hodnotu false a premenná
 * value ostáva nezmenená.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
bool bst_search(bst_node_t *tree, char key, int *value) {
  bst_node_t *item_ptr = tree;
  while(item_ptr != NULL){
    if(item_ptr->key == key){
      *value = item_ptr->value;
      return true;
    } else if(item_ptr->key > key){
      item_ptr = item_ptr->left;
    } else {
      item_ptr = item_ptr->right;
    }
  }
  return false;
}

/*
 * Vloženie uzlu do stromu.
 *
 * Pokiaľ uzol so zadaným kľúčom v strome už existuje, nahraďte jeho hodnotu.
 * Inak vložte nový listový uzol.
 *
 * Výsledný strom musí spĺňať podmienku vyhľadávacieho stromu — ľavý podstrom
 * uzlu obsahuje iba menšie kľúče, pravý väčšie.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_insert(bst_node_t **tree, char key, int value) {
  bst_node_t *item_ptr = *tree, *item_ptr_father = NULL;
  while(item_ptr != NULL){
    item_ptr_father = item_ptr;
    if(item_ptr->key == key){
      item_ptr->value = value;
      return;
    } else if(item_ptr->key > key){
      item_ptr = item_ptr->left;
    } else {
      item_ptr = item_ptr->right;
    }
  }
  
  item_ptr = malloc(sizeof(struct bst_node));
  if(item_ptr == NULL){
    fprintf(stderr,"bst_insert: MALLOC FAILURE\n");
    exit(1);
  }
  item_ptr->key = key;
  item_ptr->value = value;
  item_ptr->right = NULL;
  item_ptr->left = NULL;
  if(item_ptr_father == NULL){
    *tree = item_ptr;
  } else if(item_ptr_father->key > key){
    item_ptr_father->left = item_ptr;
  } else {
    item_ptr_father->right = item_ptr;
  }
}


/*
 * Pomocná funkcia ktorá nahradí uzol najpravejším potomkom.
 *
 * Kľúč a hodnota uzlu target budú nahradené kľúčom a hodnotou najpravejšieho
 * uzlu podstromu tree. Najpravejší potomok bude odstránený. Funkcia korektne
 * uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkcia predpokladá že hodnota tree nie je NULL.
 *
 * Táto pomocná funkcia bude využitá pri implementácii funkcie bst_delete.
 *
 * Funkciu implementujte iteratívne bez použitia vlastných pomocných funkcií.
 */
void bst_replace_by_rightmost(bst_node_t *target, bst_node_t **tree) {
  bst_node_t *item_ptr;
  stack_bst_t stack;
  stack_bst_init(&stack);

  if(target->left == NULL){
    // Neexistuji oba synove
    if(target->right == NULL){
      return;
    
    // Neexistuje pouze levy syn
    } else {
      item_ptr = target->right;
      target->key = item_ptr->key;
      target->value = item_ptr->value;
      target->left = item_ptr->left;
      target->right = item_ptr->right;
      free(item_ptr);
      return;
    }
  // Neexistuje pouze pravy syn
  } else if(target->right == NULL){
    item_ptr = target->left;
    target->key = item_ptr->key;
    target->value = item_ptr->value;
    target->left = item_ptr->left;
    target->right = item_ptr->right;
    free(item_ptr);
    return;
  }

  // Existuji oba synove
  item_ptr = target->left;
  stack_bst_push(&stack,target);

  // Nalezeni nejpravejsiho syna
  // Po vykonani na vrcholu zasobniku ukazatel na uzel, kterym budeme nahrazovat target
  while(item_ptr != NULL){
    stack_bst_push(&stack,item_ptr);
    item_ptr = item_ptr->right;
  }

  // Na item_ptr se nachazi ukazatel na uzel, kterym budeme nahrazovat target
  item_ptr = stack_bst_pop(&stack);

  // Leveho potomka uzlu, kterym budeme nahrazovat target, priradime jeho otci
  (&stack)->items[(&stack)->top]->right = item_ptr->left;

  // Nahrazujeme target
  target->key = item_ptr->key;
  target->value = item_ptr->value;

  // Uvolnime item_ptr
  free(item_ptr);
}

/*
 * Odstránenie uzlu v strome.
 *
 * Pokiaľ uzol so zadaným kľúčom neexistuje, funkcia nič nerobí.
 * Pokiaľ má odstránený uzol jeden podstrom, zdedí ho otec odstráneného uzla.
 * Pokiaľ má odstránený uzol oba podstromy, je nahradený najpravejším uzlom
 * ľavého podstromu. Najpravejší uzol nemusí byť listom!
 * Funkcia korektne uvoľní všetky alokované zdroje odstráneného uzlu.
 *
 * Funkciu implementujte iteratívne pomocou bst_replace_by_rightmost a bez
 * použitia vlastných pomocných funkcií.
 */
void bst_delete(bst_node_t **tree, char key) {
  bst_node_t *item_ptr = *tree, *ptr = NULL;
  stack_bst_t stack;
  stack_bst_init(&stack);

  while(item_ptr != NULL){
    stack_bst_push(&stack,item_ptr);
    
    // Klic nalezen
    if(item_ptr->key == key){
      bst_replace_by_rightmost(item_ptr,tree);
      
      // Jestli byl uzel opravdu odstranen
      if(item_ptr->key == key){
        ptr = stack_bst_pop(&stack);
        ptr = stack_bst_pop(&stack); // Nyni si v ptr nachazi otec niceneho uzlu
        if(ptr->right == item_ptr){ // Niceny prvek se nachazi v pravem synovi
          ptr->right = NULL;
          free(item_ptr);
        } else {
          ptr->left = NULL;
          free(item_ptr);
        }
      }
      return;

    //Klic nenalezen, pokracujeme dale ve strome
    } else if(item_ptr->key > key){
      item_ptr = item_ptr->left;
    } else {
      item_ptr = item_ptr->right;
    }
  }
}

/*
 * Zrušenie celého stromu.
 *
 * Po zrušení sa celý strom bude nachádzať v rovnakom stave ako po
 * inicializácii. Funkcia korektne uvoľní všetky alokované zdroje rušených
 * uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_dispose(bst_node_t **tree) {
  bst_node_t *item_ptr;
  item_ptr = *tree;
  stack_bst_t stack;
  stack_bst_init(&stack);

  while(*tree != NULL){
    stack_bst_push(&stack,item_ptr);
    
    // Existuje levy syn
    if(item_ptr->left != NULL){
      item_ptr = item_ptr->left;

    // Existuje pravy syn
    } else if(item_ptr->right != NULL){
      item_ptr = item_ptr->right;
    
    // Uz neexistuje zadny syn a jsme u ukazatele na koren
    } else if(item_ptr == *tree){
      free(item_ptr);
      *tree = NULL;
    
    // Uvolnujeme uzel, ktery nema zadne syny ani neni koren
    } else {
      item_ptr = stack_bst_pop(&stack);
      if((&stack)->items[(&stack)->top]->right == item_ptr){
        free(item_ptr);
        item_ptr = stack_bst_pop(&stack);
        item_ptr->right = NULL;
      } else {
        free(item_ptr);\
        item_ptr = stack_bst_pop(&stack);
        item_ptr->left = NULL;
      }
    }
  }
}

/*
 * Pomocná funkcia pre iteratívny preorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu.
 * Nad spracovanými uzlami zavola bst_print_node a uloží ich do zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_preorder(bst_node_t *tree, stack_bst_t *to_visit) {
  bst_node_t *item_ptr;
  item_ptr = tree;

  while(item_ptr != NULL){
    // Jestli existuje jeste pravy syn -> pushujeme na zasobnik
    if(item_ptr->right != NULL){
      stack_bst_push(to_visit,item_ptr->right);
    }

    // Jestli ma leveho syna
    bst_print_node(item_ptr);
    item_ptr = item_ptr->left;
  }
}

/*
 * Preorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_preorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_preorder(bst_node_t *tree) {
  bst_node_t *item_ptr;
  item_ptr = tree;
  stack_bst_t stack;
  stack_bst_init(&stack);

  while(true){
    bst_leftmost_preorder(item_ptr,&stack);
    if((&stack)->top == -1){
      break;
    }
    item_ptr = stack_bst_pop(&stack);
  }
}

/*
 * Pomocná funkcia pre iteratívny inorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov.
 *
 * Funkciu implementujte iteratívne pomocou zásobníku uzlov a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_inorder(bst_node_t *tree, stack_bst_t *to_visit) {
  bst_node_t *item_ptr;
  item_ptr = tree;

  while(item_ptr != NULL){
    stack_bst_push(to_visit,item_ptr);
    item_ptr = item_ptr->left;
  }
}

/*
 * Inorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_inorder a
 * zásobníku uzlov bez použitia vlastných pomocných funkcií.
 */
void bst_inorder(bst_node_t *tree) {
  bst_node_t *item_ptr;
  item_ptr = tree;
  stack_bst_t stack;
  stack_bst_init(&stack);

  bst_leftmost_inorder(item_ptr,&stack);

  while((&stack)->top >= 0){
    item_ptr = stack_bst_pop(&stack);
    bst_print_node(item_ptr);
    if(item_ptr->right != NULL){
      item_ptr = item_ptr->right;
      bst_leftmost_inorder(item_ptr,&stack);
    }
  }
}

/*
 * Pomocná funkcia pre iteratívny postorder.
 *
 * Prechádza po ľavej vetve k najľavejšiemu uzlu podstromu a ukladá uzly do
 * zásobníku uzlov. Do zásobníku bool hodnôt ukladá informáciu že uzol
 * bol navštívený prvý krát.
 *
 * Funkciu implementujte iteratívne pomocou zásobníkov uzlov a bool hodnôt a bez použitia
 * vlastných pomocných funkcií.
 */
void bst_leftmost_postorder(bst_node_t *tree, stack_bst_t *to_visit,
                            stack_bool_t *first_visit) {
  bst_node_t *item_ptr;
  item_ptr = tree;

  while(item_ptr != NULL){
    stack_bst_push(to_visit,item_ptr);
    stack_bool_push(first_visit,true);
    item_ptr = item_ptr->left;
  }
}

/*
 * Postorder prechod stromom.
 *
 * Pre aktuálne spracovávaný uzol nad ním zavolajte funkciu bst_print_node.
 *
 * Funkciu implementujte iteratívne pomocou funkcie bst_leftmost_postorder a
 * zásobníkov uzlov a bool hodnôt bez použitia vlastných pomocných funkcií.
 */
void bst_postorder(bst_node_t *tree) {
  bst_node_t *item_ptr;
  item_ptr = tree;
  bool visit_done;
  
  stack_bst_t stack;
  stack_bst_init(&stack);

  stack_bool_t visited;
  stack_bool_init(&visited);

  bst_leftmost_postorder(item_ptr,&stack,&visited);

  while((&stack)->top >= 0){
    item_ptr = stack_bst_pop(&stack);
    visit_done = stack_bool_pop(&visited);
    if(item_ptr->right != NULL && visit_done == true){
      stack_bst_push(&stack,item_ptr);
      stack_bool_push(&visited,false);
      bst_leftmost_postorder(item_ptr->right,&stack,&visited);
    } else {
      bst_print_node(item_ptr);
    }


  }
}
