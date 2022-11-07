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
  item_ptr = target;
  stack_bst_t stack;
  stack_bst_init(&stack);

  // Jdeme hledat do leveho podstromu
  if(item_ptr->left != NULL){
    //Levy syn existuje
    item_ptr = item_ptr->left;
  } else {
    // Neni zadny levy syn neexistuje
    if(target->right != NULL){
      // Pravy syn existuje, muzeme pokracovat nahrazovanim
      item_ptr = item_ptr->right;
      target->key = target->right->key;
      target->value = target->right->value;
      target->right = target->right->right;
      target->left = target->right->left;
      free(item_ptr);
    } else {
      // Zadny syn neexistuje, pouze rusime target
      free(target);
      return;
    }
  }

  // Pushneme si syny ruseneho uzlu
  stack_bst_push(&stack,target->left);
  stack_bst_push(&stack,target->right); //Z: ..., T->L, T->R

  // Hledame nejpravejsi uzel leveho podstromu
  while(item_ptr->right != NULL){
    if(item_ptr->right->right == NULL){
      // Dalsi pravy syn po pravem synovi je NULL
      stack_bst_push(&stack,item_ptr->right); //Z: ..., T->L, T->R, XXXX
      item_ptr->right = item_ptr->right->left;
    }
    item_ptr = item_ptr->right;
  }

  // Nahrazujeme target
  target->key = (&stack)->items[(&stack)->top]->key;
  target->value = (&stack)->items[(&stack)->top]->value;
  item_ptr = stack_bst_pop(&stack); //Z: ..., T->L, T->R
  free(item_ptr);
  item_ptr = stack_bst_pop(&stack); //Z: ..., T->L
  target->right = item_ptr;
  item_ptr = stack_bst_pop(&stack); //Z: ...
  target->left = item_ptr;
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
  /*stack_bst_t *stack;
  stack_bst_init(stack);
  bst_node_t *record;
  record = tree;
  while(record != NULL){
    bst_print_node(record); // Tisk aktualne zpracovavaneho prvku

  }*/
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
}
